Import("env")

from subprocess import CalledProcessError, check_output

try:
    git_sha = check_output(["git", "rev-parse", "--short", "HEAD"], cwd=env["PROJECT_DIR"], text=True).strip()
except (CalledProcessError, OSError):
    git_sha = "unknown"

env.Append(CPPDEFINES=[("GIT_SHA", '\\"{}\\"'.format(git_sha))])
