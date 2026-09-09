# Prompt for generating prompt
ROLE:
you are a prompt engineer that will create a prompt for cursor coding agent.
you are not to add detail about spec that the user does NOT asks you to include or find. your job is to get details the user asks you to find and restructure user request into clear and non conflicting prompt. 

USER:

create new feature. this feature is a CRUD to person table in SQLite DB @db.db 

the database table is already there
you should check the current db person table and bake that detail into the prompt

I want the prompt to scope work to `person` endpoints only
dont fix auth and user. should be stated to be out of scope.
this should be new feature run and not refactor / fixing run.

this endpoint should follow clean architecture. 
clean architecture layers is model, service and controllers :
- controllers: this is the api front should only validate request and auth see @includes/controllers/User.cpp User::getUserById of how auth implemented
- service: this layer abstract the model and only provide what contollers needs to create, read, update and delete
- model: create a class that represent that table. (repository agnostic)
- repo: separate from model but coupled. this is where the runtime sql lives. can be inside data or model (let me know which one is best and why)

let me know if above request is conflicting with current project structure so I will decide what to put in the final prompt.
asks me first if more detail needed to be included in the prompt before inluding it

write the prompt in the file AI_PROMPT_README.md in root dir.

---
# Task

Implement a **new feature**: CRUD for the existing SQLite `person` table.

This is a **new-feature run**, not a refactor or fix run. Do not change, clean up, or "improve" existing Auth or User behavior.

## Scope

**In scope**
- Person endpoints and the Person model / service / controller / repo needed to support them
- Create new Person files; there is no existing Person controller, stub, or source to reuse
- Wire the new Person app into routing and the build

**Out of scope**
- Auth (`includes/controllers/Auth.*`)
- User (`includes/controllers/User.*`)
- Do not fix, refactor, or complete Auth or User

## Database (already exists — do not migrate or redesign)

SQLite file: `db.db`

```sql
CREATE TABLE IF NOT EXISTS "person" (
	"personId"	INTEGER NOT NULL,
	"personName"	TEXT NOT NULL,
	"personEmail"	TEXT NOT NULL UNIQUE,
	"personAddress"	TEXT,
	PRIMARY KEY("personId" AUTOINCREMENT)
);
```

Columns:
- `personId` INTEGER NOT NULL, PRIMARY KEY AUTOINCREMENT
- `personName` TEXT NOT NULL
- `personEmail` TEXT NOT NULL UNIQUE
- `personAddress` TEXT (nullable)

The table is already there. Do not change the schema.

## Architecture (Person feature only)

Use these layers for Person. Do not introduce these layers into User or Auth.

### Controllers (`includes/controllers/`)

API front. Only:
- validate the request
- check auth

Auth must follow the same pattern as `User::getUserById` in `includes/controllers/User.cpp`:
- read token from `request().getenv("HTTP_TOKEN")`
- validate with `TokenManager::getInstance().isValidToken(token)`

Copy that pattern. Do not change TokenManager, and do not fix User.

### Service

Abstracts the model. Expose only what the Person controller needs for create, read, update, and delete. Controllers must not talk to the model or repo directly.

### Model

A class that represents the `person` table. Repository-agnostic: no SQL in the model.

### Repo (`includes/data/`)

Separate from the model but coupled to it. Runtime SQL for `person` lives here (not in the controller, not in the model). Put Person repo files under `includes/data/` (this project already keeps DB types there: Master, Connection).

## Wiring required for the new feature

- Register the Person controller in `main.cpp` (do not alter existing User/Auth attach blocks except as needed to add Person)
- Add new `.cpp` files to `API_SRC` in `CMakeLists.txt`

## Constraints

- Do not refactor existing features to match this architecture
- Do not put Person SQL in the controller or in the model
- Do not invent a new `person` table; use the schema above

---

# Prompt for generating prompt
ROLE:
you are a prompt engineer that will create a prompt for cursor coding agent.
you are not to add detail about spec that the user does NOT asks you to include or find. your job is to get details the user asks you to find and restructure user request into clear and non conflicting prompt. 

USER:

bring in all files that makes up Person CRUD feature in all layer for context:
- controller
- services
- models
- data

create a prompt for coding-agent to generate unittest and smoke test script that will be run by human to evaluate and debug. 

smoketest script requirement:
scripts/person-smoketest.sh will mainly smoke test the person REST endpoint on running localhost dev server.
the script should not start the server.
default base path is http://localhost:8080/ and semi configurable 
use something like `BASE_URL="${BASE_URL:-http://localhost:8080}"`
so
BASE_URL="https://mypublicdomain.com" ./scripts/person-smoketest.sh to load different base url
read current @includes/controllers/Auth.cpp and @includes/controllers/User.cpp to know the current auth process.
print pretty json response if possible.
use python to do a pretty json print.
print raw response body when python not available.
stop the script with a clear error when curl is not available

unittest requirement:
unittest scope:
  - full happy path: HTTP Request -> controllers::Person -> PersonService -> models::Person -> PersonRepo -> test.db
  - controller errors (unauthorized, malformed requests, duplicated email, accessing deleted)
out of scope:
  - person services isolation test 
  - person rep isolation test

use temporary test.db for the tests. 
unittest should delete if that db exits and crate new.


write your prompt by appending it into @AI_PROMPT_README.md 

---

# Task

Generate a **unit test** and a **smoke test script** for the existing Person CRUD feature. A human will run both to evaluate and debug. Do not change Person CRUD behavior, Auth, or User.

## Required context (read these; do not change them)

Person CRUD layers:

- controller: `includes/controllers/Person.h`, `includes/controllers/Person.cpp`
- service: `includes/services/PersonService.h`, `includes/services/PersonService.cpp`
- model: `includes/models/Person.h`
- data: `includes/data/PersonRepo.h`, `includes/data/PersonRepo.cpp`

Auth process (current implementation — use as-is, do not fix):

- `includes/controllers/Auth.cpp` — `POST /auth/login` expects JSON `{"User":{"LoginId":"...","Password":"..."}}`. Success body is `User accepted`. Login does **not** issue a token (`TODO: add jwt token`).
- `includes/controllers/User.cpp` — token is the `Token` HTTP header (`request().getenv("HTTP_TOKEN")`). `User::getUser` (`GET /users`) **registers** that header value into `TokenManager` via `addToken`. `User::getUserById` **validates** with `TokenManager::getInstance().isValidToken(token)`.
- Person auth matches `User::getUserById`: `Person::requireAuth` reads `HTTP_TOKEN` and calls `isValidToken`. Unauthorized writes `{"error":"Unauthorized"}`.

Person REST routes (from Person dispatcher + `main.cpp` attach `/person`):

- `GET /person` — list
- `POST /person` — create
- `GET /person/{id}` — get by id
- `PUT /person/{id}` — update
- `DELETE /person/{id}` — delete

JSON body for create/update: `personName` (required non-empty string), `personEmail` (required non-empty string), `personAddress` (optional string or null).

## Smoke test script

Create `scripts/person-smoketest.sh`.

- Smoke test the Person REST endpoints on an **already running** localhost (or other) server.
- The script must **not** start the server.
- Default base URL, overridable:

```bash
BASE_URL="${BASE_URL:-http://localhost:8080}"
```

Example:

```bash
BASE_URL="https://mypublicdomain.com" ./scripts/person-smoketest.sh
```

- Use the current auth process above (do not invent JWT login). Authenticate the way Auth.cpp / User.cpp actually work today, then call Person endpoints.
- If `curl` is not available, stop with a clear error.
- Pretty-print JSON responses with Python when Python is available.
- If Python is not available, print the raw response body.

## Unit test

**In scope**

- Full happy path through the stack: HTTP Request → `controllers::Person` → `PersonService` → `models::Person` → `PersonRepo` → `test.db`
- Controller errors:
  - unauthorized
  - malformed requests
  - duplicated email
  - accessing deleted

**Out of scope**

- PersonService isolation tests
- PersonRepo isolation tests

**test.db**

- Use a temporary `test.db` for the unit tests.
- If `test.db` already exists, delete it, then create a new one.

Do not add PersonService-only or PersonRepo-only tests.

---

# Task

Add a **CMake deploy bundle** so a human can build a runtime tarball on the build machine, copy it to a VM that has **no git repo**, extract it, and start the app with Compose (`docker compose` or `podman compose`). This is a packaging/deploy run, not a feature or refactor of Person/Auth/User.

## Goal

- `cmake --build … --target bundle` (or equivalent `make bundle`) produces a self-contained runtime directory **and** a tarball.
- The VM receives only that archive. It must not need source, `includes/`, CMake, or the full repo.
- The container runtime must **not** compile C++ (`cmake` / `make` / compilers must not appear in the image build). Prefer a stock Fedora image plus bind-mounted artifacts.
- Compose must be **engine-agnostic**: work with Docker Compose **and** Podman Compose. Do not use Docker-only APIs, Docker Swarm keys, or `docker` in command names inside the compose file. A `Dockerfile` is allowed only if it is also valid for Podman; prefer **no image build** (use `image:` + volumes) unless packages are missing at runtime.
- Do not make the docs or filenames exclusive to Docker. Say “container” / “Compose”. Mention both `docker compose up` and `podman compose up` as equivalent ways to run it.

## In scope

- CMake `install()` rules for the runtime layout
- A `bundle` custom target that stages `install()` into a directory under the build tree and creates a `.tar.gz`
- Linux **relative rpath** (`$ORIGIN/lib`) on `cppcms_simple` so the binary finds bundled libs next to itself after extract (not hardcoded `/home/…` or `/usr/local/lib64`)
- Compose file in the bundle that runs the prebuilt binary
- `config.json` listen address usable from outside the container (`ip` `0.0.0.0`)
- Brief README section (or a short `BUNDLE.md` in the bundle) for extract + compose up on the VM

## Out of scope

- Person / Auth / User behavior, tests, or smoke scripts
- Multi-stage compile-inside-container
- CI, SSH, or a script that copies to the VM
- Changing cppdb / CppCMS source
- Windows/macOS bundle (Linux x86-64 only; this app is built on Fedora)

## Bundle layout (what the tarball contains)

```text
cppcms_simple          # the executable
config.json
db.db.example           # seed DB only; do not name it db.db in the tarball
lib/                   # libcppcms, libbooster, libcppdb (real .so files + SONAME links)
compose.yaml
```

Optional: a tiny `Dockerfile` **only if** the stock `fedora:44` image is missing `sqlite-libs` or `pcre`. That Dockerfile may `dnf install` runtime RPMs only — no compilers.

Do **not** ship `db.db` as the live database filename inside the archive. Ship `db.db.example`. Document: on first deploy, `cp db.db.example db.db`; later deploys must not overwrite an existing `db.db`.

## CMake

Keep existing targets (`cppcms_simple`, `person_tests`). Add install + bundle; do not break the current local debug flow.

**Install**

- `install(TARGETS cppcms_simple RUNTIME DESTINATION .)`
- `install(FILES config.json compose.yaml …)`
- Install shared libraries into `lib/`:
  - `${LIB_BOOSTER}` / `${LIB_CPPCMS}` (whatever `find_library` resolved)
  - `third_party/cppdb/lib/libcppdb.so*` (Linux `.so` only, not `.dylib`)
- Install `db.db` as `db.db.example`

**`bundle` target**

- Depends on `cppcms_simple`
- `cmake --install ${CMAKE_BINARY_DIR} --prefix <build>/bundle` (or equivalent staging dir)
- Produce `<build>/cppcms-simple-bundle.tar.gz`
- Do not require CPack unless it is simpler; `install()` + `add_custom_target(bundle)` is enough

**Rpath (Linux)**

Today the link line embeds:

```text
-Wl,-rpath,/home/…/third_party/cppdb/lib:/usr/local/lib64
```

That comes from `link_directories(${THIRD_PARTY_DIR}/cppdb/lib)` plus CMake’s default rpath for libs found in `/usr/local/lib64`. It is not a string in `main.cpp`.

For `cppcms_simple` on Linux:

- Set `BUILD_RPATH` / `INSTALL_RPATH` to `$ORIGIN/lib`
- Set `BUILD_RPATH_USE_ORIGIN` / `BUILD_WITH_INSTALL_RPATH` as needed so the **build-tree binary** already uses `$ORIGIN/lib`, not the source-tree absolute path
- Stop relying on `link_directories` for cppdb if that re-injects an absolute rpath; link `cppdb` by full path or an `IMPORTED` library instead
- Leave the existing `if(APPLE)` `install_name_tool` block as-is (macOS only)

`person_tests` may keep an absolute `BUILD_RPATH` for local tests; do not require tests to be in the bundle.

After the change, `readelf -d build/cppcms_simple` RUNPATH/RPATH must contain `$ORIGIN/lib` (or equivalent), not the developer home directory.

## Compose (engine-agnostic)

Put `compose.yaml` at repo root so `install()` can copy it into the bundle (or generate it into the bundle; one compose file is enough).

- `image: fedora:44` (match the Fedora 44 x86-64 build host / ICU 77 / glibc)
- `working_dir: /app`
- Bind-mount the extracted files: binary, `config.json`, `db.db`, `lib/` → `/usr/local/lib64` or `/app/lib`
- Set `LD_LIBRARY_PATH` to the mounted lib dir as a safety net even with `$ORIGIN`
- Publish `8080:8080`
- `command` runs `./cppcms_simple -c config.json`
- Persist `db.db` via a bind mount of the file next to compose (the file the operator created from `db.db.example`)
- Avoid Docker-only fields. If a volume option is needed for rootless Podman+SELinux, prefer a comment in the README (`:Z`) rather than making SELinux the default for Docker Desktop.

`config.json` used at runtime must listen on all interfaces:

```json
"service": {
  "api": "http",
  "port": 8080,
  "ip": "0.0.0.0"
}
```

Keep local-dev `config.json` working: either add `"ip": "0.0.0.0"` to the existing file (acceptable; still binds 8080) or install a container-specific `config.json` into the bundle only. Do not break `-c config.json` for local `./cppcms_simple`.

## Constraints

- Do not put the git repo in the bundle
- Do not compile in the container
- Do not copy macOS `.dylib` files into `lib/`
- Do not overwrite a live `db.db` on redeploy via the tarball contents
- Do not change Person/Auth/User source except if `config.json` listen IP is updated as above
- `.gitignore`: ignore the staging dir / tarball if they land in the repo (e.g. `build/` is already ignored)

## Done when

- `cmake --build build --target bundle` creates the tarball
- Extracting the tarball on a machine with Compose + `fedora:44` image is enough to run the API on port 8080
- The binary’s rpath is relative (`$ORIGIN/lib`), not a host absolute path
- Compose instructions work for both Docker and Podman

