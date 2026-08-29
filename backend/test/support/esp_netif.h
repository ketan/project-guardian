#pragma once

#include <cstdint>

using esp_err_t = int;
constexpr esp_err_t ESP_OK = 0;

struct esp_netif_t {};
struct esp_ip6_addr_t { uint32_t addr[4]{}; };

inline esp_netif_t *esp_netif_get_handle_from_ifkey(const char *) { return nullptr; }
inline esp_err_t esp_netif_get_ip6_global(esp_netif_t *, esp_ip6_addr_t *) { return -1; }
