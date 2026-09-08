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
