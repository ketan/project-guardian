#!/usr/bin/env bash

set -euo pipefail

BASE_URL="${BASE_URL:-http://192.168.1.33}"
TOKEN="${TOKEN:-secret}"
ENDPOINT="${ENDPOINT:-/api/v1/status}"
TOTAL_REQUESTS="${TOTAL_REQUESTS:-100}"
CONCURRENCY="${CONCURRENCY:-8}"

if [[ "${1:-}" == "--help" ]]; then
  cat <<'EOF'
Simple API load test for ESP32 AsyncWebServer.

Environment variables:
  BASE_URL        default: http://192.168.1.33
  TOKEN           default: secret
  ENDPOINT        default: /api/v1/status
  TOTAL_REQUESTS  default: 100
  CONCURRENCY     default: 8

Example:
  BASE_URL=http://192.168.1.33 TOKEN=secret ENDPOINT=/api/v1/status TOTAL_REQUESTS=200 CONCURRENCY=10 \
  ./backend/scripts/load_test_api.sh
EOF
  exit 0
fi

if ! [[ "$TOTAL_REQUESTS" =~ ^[0-9]+$ ]] || ! [[ "$CONCURRENCY" =~ ^[0-9]+$ ]]; then
  echo "TOTAL_REQUESTS and CONCURRENCY must be positive integers" >&2
  exit 1
fi

if (( TOTAL_REQUESTS <= 0 || CONCURRENCY <= 0 )); then
  echo "TOTAL_REQUESTS and CONCURRENCY must be > 0" >&2
  exit 1
fi

if (( CONCURRENCY > TOTAL_REQUESTS )); then
  CONCURRENCY="$TOTAL_REQUESTS"
fi

AB_OUTPUT_FILE="$(mktemp)"
trap 'rm -f "$AB_OUTPUT_FILE"' EXIT

echo "Running load test"
echo "  URL:         ${BASE_URL}${ENDPOINT}"
echo "  Requests:    ${TOTAL_REQUESTS}"
echo "  Concurrency: ${CONCURRENCY}"
echo

if ! command -v ab >/dev/null 2>&1; then
  echo "ab not found. Install ApacheBench." >&2
  exit 1
fi

ab \
  -n "$TOTAL_REQUESTS" \
  -c "$CONCURRENCY" \
  -H "Authorization: Bearer ${TOKEN}" \
  "${BASE_URL}${ENDPOINT}" >"$AB_OUTPUT_FILE"

complete="$(awk -F': *' '/^Complete requests:/ { print $2 }' "$AB_OUTPUT_FILE" | tr -d ' ')"
failed="$(awk -F': *' '/^Failed requests:/ { print $2 }' "$AB_OUTPUT_FILE" | tr -d ' ')"
non_2xx="$(awk -F': *' '/^Non-2xx responses:/ { print $2 }' "$AB_OUTPUT_FILE" | tr -d ' ')"
if [[ -z "$non_2xx" ]]; then
  non_2xx="0"
fi
rps="$(awk -F': *' '/^Requests per second:/ { print $2 }' "$AB_OUTPUT_FILE" | awk '{ print $1 }')"
total_s="$(awk -F': *' '/^Time taken for tests:/ { print $2 }' "$AB_OUTPUT_FILE" | awk '{ print $1 }')"
avg_ms="$(awk -F': *' '/^Time per request:/ { print $2 }' "$AB_OUTPUT_FILE" | awk 'NR==1 { print $1 }')"
max_ms="$(awk '/^ *100%/ { print $2 }' "$AB_OUTPUT_FILE")"
p50_ms="$(awk '/^ *50%/ { print $2 }' "$AB_OUTPUT_FILE")"
p95_ms="$(awk '/^ *95%/ { print $2 }' "$AB_OUTPUT_FILE")"
p99_ms="$(awk '/^ *99%/ { print $2 }' "$AB_OUTPUT_FILE")"

echo "Results"
echo "  Total completed: ${complete}/${TOTAL_REQUESTS}"
echo "  Failed requests: ${failed}"
echo "  Non-2xx:         ${non_2xx}"
echo "  Throughput:      ${rps} req/s"
echo "  Total duration:  ${total_s} s"
echo "  Latency avg/max: ${avg_ms} / ${max_ms} ms"
echo "  Latency p50/p95/p99: ${p50_ms} / ${p95_ms} / ${p99_ms} ms"
