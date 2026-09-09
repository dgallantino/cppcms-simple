Research REST API with [CppCMS](http://cppcms.com/wikipp/en/page/main)

[Read more](http://cppcms.com/wikipp/en/page/cppcms_1x#Tutorials)

### How to run on Mac

1. Make sure C++ already active (using XCode)
2. Install cppcms `brew install cppcms`
3. Install cmake `brew install cmake`
4. Run `mkdir build`
5. Run `cmake ..`
6. Run `make`
7. Run `cp ../db.db .` to copy the database example
8. After executable `cppcms_simple` built, then you can run it by command `./cppcms_simple -c ../config.json`

Note:

- CPPDB on the third-party submodules are built with MacOS M1 CPU, if you are run with other Arch maybe return error
- Make sure your database connection is correct on the file `config.json`

If those steps are not working due to cppdb dynamic libs, do this workaround to run the application.
1. Clone the repository https://github.com/saypulung/cppdb
2. In the terminal, go to the directory cppdb source code. For example **/Users/pulung/cppdb**
3. Run `mkdir build && cd build`
4. Run `cmake ..`
5. Run `make`
6. Edit CMakeList.txt file in the line 30. Replace `link_directories(${THIRD_PARTY_DIR}/cppdb/lib)` to `link_directories(/Users/pulung/cppdb/build)`. (Note: /Users/pulung/cppdb is the directory that you located the cppdb library
7. Go to `cppcms-simple` directory and rebuild the apps, once you've success to compile, it should be run propperly.

### How to run on Linux

Build and run locally (Fedora 44 x86-64):

1. Install CppCMS/Booster into `/usr/local` (or another prefix `find_library` can see)
2. `mkdir -p build && cmake -S . -B build && cmake --build build`
3. `./build/cppcms_simple -c config.json`

### Deploy with Compose (no git on the VM)

On the local **build** machine:

```sh
cmake -S . -B build
cmake --build build --target bundle
```

That writes `build/bundle/` and `build/cppcms-simple-bundle.tar.gz`. Copy only the tarball to the VM.

On the VM (Docker Compose or Podman Compose; both are equivalent):

```sh
mkdir -p cppcms-simple
tar -xzf cppcms-simple-bundle.tar.gz -C cppcms-simple
cd cppcms-simple
cp db.db.example db.db   # first deploy only; do not overwrite an existing db.db
docker compose up --build
# or: podman compose up --build
```

See `BUNDLE.md` inside the archive for SELinux notes. The container image does not compile C++.

### How to run on Windows

- TODO LIST

### Debugging

- Run the project first by `How to run on Mac` section (if you are using Mac OS M1) and replace command at step 5 with `cmake -DCMAKE_BUILD_TYPE=Debug ..`
- Set Breakpoint which you want to debug like this (User.cpp:24)
![Set breakpoint](/screenshots/breakpoint.png)
- On the VSCode, you can run by click Play Button at **Run and Debug** section
- Then, after you are ran the application and accessing the URL target method, program will paused at the breakpoint
![Paused](/screenshots/paused.png)

### Tests

#### Unit tests

Does not need the app running.

1. `mkdir -p build && cmake -S . -B build`
2. `cmake --build build --target api_tests`
3. `./build/api_tests`

Or `ctest --test-dir build --output-on-failure`

#### Smoketest

Needs a running app (`./build/cppcms_simple -c config.json`) and `curl`.

1. `./scripts/api-smoketest.sh`

Note:

- Default URL is `http://localhost:8080`. Override with `BASE_URL`: `BASE_URL=http://127.0.0.1:8080 ./scripts/api-smoketest.sh`
- `-v` (or `--verbose`) prints request/response bodies: `./scripts/api-smoketest.sh -v`

### Available endpoints

Base URL: `http://localhost:8080` (see `config.json`). All request and response bodies are `application/json`.

Protected routes (`/users`, `/person`, and `POST /auth/logout`) require the `Token` header with a value returned by `POST /auth/login`. Missing or invalid tokens return `401`:

```json
{"error":"Unauthorized"}
```

Other errors use the same `{"error":"<message>"}` shape.

#### Auth (`/auth`)

| Method | Path | Auth | Description |
| --- | --- | --- | --- |
| `POST` | `/auth/login` | no | Exchange login credentials for a token |
| `POST` | `/auth/logout` | `Token` header | Invalidate the current token |

**`POST /auth/login`**

Request:

```json
{
  "User": {
    "LoginId": "admin",
    "Password": "admin1234"
  }
}
```

`User`, `LoginId`, and `Password` are required non-empty strings.

| Status | When |
| --- | --- |
| `200` | Credentials match. Body: `{"token":"<opaque token>"}` |
| `400` | Empty body (`JSON body is required`), invalid JSON (`JSON Invalid`), missing `User` / `LoginId` / `Password` |
| `401` | Unknown login or wrong password (`Login invalid`) |
| `405` | Method is not `POST` |

**`POST /auth/logout`**

No body. Send the token in the `Token` header.

| Status | When |
| --- | --- |
| `200` | Token revoked. Body: `{"ok":true}` |
| `401` | Missing or unknown token |
| `405` | Method is not `POST` |

After logout, the same token is rejected on `/users` and `/person`.

#### Users (`/users`)

User accounts (login ids), not people. Every method requires a valid `Token`.

| Method | Path | Description |
| --- | --- | --- |
| `GET` | `/users` | List all users |
| `POST` | `/users` | Create a user (`201`) |
| `GET` | `/users/{id}` | Get one user by numeric `usrsId` |
| `PUT` | `/users/{id}` | Replace a user |
| `DELETE` | `/users/{id}` | Delete a user |

Create / update body:

```json
{
  "usrsFirstName": "Smoke",
  "usrsLastName": "Test",
  "usrsLoginId": "smokeuser",
  "usrsLoginPass": "smoke1234"
}
```

| Field | Required | Notes |
| --- | --- | --- |
| `usrsLoginId` | yes | Non-empty string; unique |
| `usrsLoginPass` | yes | Non-empty string |
| `usrsFirstName` | no | String or `null` (stored as empty string) |
| `usrsLastName` | no | String or `null` (stored as empty string) |

Response object (list items are the same shape):

```json
{
  "usrsId": 1,
  "usrsFirstName": "Smoke",
  "usrsLastName": "Test",
  "usrsLoginId": "smokeuser",
  "usrsLoginPass": "smoke1234",
  "usrsCreatedTime": "2024-07-01 00:00:00",
  "usrsUpdatedTime": ""
}
```

`DELETE` success body: `{"ok":true}`.

| Status | When |
| --- | --- |
| `200` | List, get, update, or delete succeeded |
| `201` | Create succeeded |
| `400` | Empty body, invalid JSON, missing `usrsLoginId` / `usrsLoginPass`, or non-string name fields |
| `401` | Missing or invalid `Token` |
| `404` | Unknown `{id}` (`User not found`) |
| `405` | Unsupported method on that path (collection allows `GET`/`POST`; item allows `GET`/`PUT`/`DELETE`) |
| `409` | `usrsLoginId already exists` |

#### Person (`/person`)

Every method requires a valid `Token`.

| Method | Path | Description |
| --- | --- | --- |
| `GET` | `/person` | List all people |
| `POST` | `/person` | Create a person (`201`) |
| `GET` | `/person/{id}` | Get one person by numeric `personId` |
| `PUT` | `/person/{id}` | Replace a person |
| `DELETE` | `/person/{id}` | Delete a person |

Create / update body:

```json
{
  "personName": "Smoke Test",
  "personEmail": "smoke@example.com",
  "personAddress": "1 Smoke St"
}
```

| Field | Required | Notes |
| --- | --- | --- |
| `personName` | yes | Non-empty string |
| `personEmail` | yes | Non-empty string; unique |
| `personAddress` | no | String, or `null` / omitted to store SQL `NULL` |

Response object (`personAddress` is omitted when null):

```json
{
  "personId": 1,
  "personName": "Smoke Test",
  "personEmail": "smoke@example.com",
  "personAddress": "1 Smoke St"
}
```

`DELETE` success body: `{"ok":true}`.

| Status | When |
| --- | --- |
| `200` | List, get, update, or delete succeeded |
| `201` | Create succeeded |
| `400` | Empty body, invalid JSON, missing `personName` / `personEmail`, or `personAddress` not a string/`null` |
| `401` | Missing or invalid `Token` |
| `404` | Unknown `{id}` (`Person not found`) |
| `405` | Unsupported method on that path (collection allows `GET`/`POST`; item allows `GET`/`PUT`/`DELETE`) |
| `409` | `personEmail already exists` |

**Example (login, then list people):**

```sh
TOKEN=$(curl -sS -X POST http://localhost:8080/auth/login \
  -H 'Content-Type: application/json' \
  -d '{"User":{"LoginId":"admin","Password":"admin1234"}}' \
  | python3 -c 'import json,sys; print(json.load(sys.stdin)["token"])')

curl -sS http://localhost:8080/person -H "Token: ${TOKEN}"
curl -sS http://localhost:8080/users -H "Token: ${TOKEN}"
```

