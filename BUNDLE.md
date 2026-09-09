# Runtime bundle

This archive is the compiled API plus shared libraries. The VM does not need git, source, or CMake. The container does not compile C++.

## First deploy

```sh
mkdir -p cppcms-simple
tar -xzf cppcms-simple-bundle.tar.gz -C cppcms-simple
cd cppcms-simple
cp db.db.example db.db
```

Create `db.db` **before** Compose starts. Bind-mounting a missing `db.db` can create a directory of that name instead of a SQLite file.

## Start

Either engine works; they are equivalent:

```sh
docker compose up --build
# or
podman compose up --build
```

`--build` installs runtime RPMs (`pcre`, `libicu`) on top of `fedora:44`. The app binary is bind-mounted; it is not compiled in the image.

The API listens on port **8080** (`0.0.0.0` inside the container).

## Later deploys

Extract the new tarball over the same directory (or beside it). **Do not** replace a live `db.db` with `db.db.example`. The tarball never contains `db.db`.

## Rootless Podman and SELinux

On Fedora with SELinux, bind mounts often fail until you add `:Z`. Leave `:Z` off on Docker Desktop and other hosts that do not use SELinux.

```yaml
volumes:
  - ./cppcms_simple:/app/cppcms_simple:ro,Z
  - ./config.json:/app/config.json:ro,Z
  - ./db.db:/app/db.db:Z
  - ./lib:/app/lib:ro,Z
```

Extract into a dedicated directory (home or `/var/tmp`). A `/tmp` extract can get `user_tmp_t` labels that the container cannot execute.
