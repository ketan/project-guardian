# Repository Conventions

## C++

- Keep each class in a file whose basename matches the class name.
- Example: `ApiServer` should live in `ApiServer.h` and `ApiServer.cpp`.
- Avoid introducing new class/file pairs where the filename and class name differ only by style or abbreviation.
- Prefer module-style files only when there is no single dominant class or type.
- Always pin backend dependency versions exactly in `platformio.ini`; do not leave firmware/library dependency versions floating.
- Add backend logging for all meaningful actions and state changes.
- Use the `RemoteDebug` library for backend logging instead of inventing a custom logging mechanism.
- Prefer streamed JSON responses over building large temporary `String` bodies in memory.
- Run `pio run` after backend changes.
