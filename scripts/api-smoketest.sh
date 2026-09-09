#!/usr/bin/env bash
set -euo pipefail

BASE_URL="${BASE_URL:-http://localhost:8080}"
BASE_URL="${BASE_URL%/}"
VERBOSE="${VERBOSE:-0}"

usage() {
    echo "Usage: $0 [-v|--verbose]"
    echo "  BASE_URL defaults to http://localhost:8080"
    echo "  VERBOSE=1 also enables request/response body printing"
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        -v|--verbose)
            VERBOSE=1
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "error: unknown argument: $1" >&2
            usage >&2
            exit 2
            ;;
    esac
done

if ! command -v curl >/dev/null 2>&1; then
    echo "error: curl is required but was not found in PATH" >&2
    exit 1
fi

pretty_json() {
    local body="$1"
    if command -v python3 >/dev/null 2>&1; then
        printf '%s\n' "$body" | python3 -m json.tool 2>/dev/null || printf '%s\n' "$body"
    elif command -v python >/dev/null 2>&1; then
        printf '%s\n' "$body" | python -m json.tool 2>/dev/null || printf '%s\n' "$body"
    else
        printf '%s\n' "$body"
    fi
}

extract_json_str() {
    local body="$1"
    local field="$2"
    if command -v python3 >/dev/null 2>&1; then
        printf '%s' "$body" | python3 -c 'import json,sys; print(json.load(sys.stdin)[sys.argv[1]])' "$field"
    elif command -v python >/dev/null 2>&1; then
        printf '%s' "$body" | python -c 'import json,sys; print(json.load(sys.stdin)[sys.argv[1]])' "$field"
    else
        printf '%s' "$body" | sed -n "s/.*\"${field}\":\"\\([^\"]*\\)\".*/\\1/p" | head -n 1
    fi
}

extract_json_int() {
    local body="$1"
    local field="$2"
    if command -v python3 >/dev/null 2>&1; then
        printf '%s' "$body" | python3 -c 'import json,sys; print(int(json.load(sys.stdin)[sys.argv[1]]))' "$field"
    elif command -v python >/dev/null 2>&1; then
        printf '%s' "$body" | python -c 'import json,sys; print(int(json.load(sys.stdin)[sys.argv[1]]))' "$field"
    else
        printf '%s' "$body" | sed -n "s/.*\"${field}\":[[:space:]]*\\([0-9][0-9]*\\).*/\\1/p" | head -n 1
    fi
}

dump_bodies() {
    local req_body="$1"
    local resp_body="$2"
    if [[ -n "$req_body" ]]; then
        echo "request body:"
        pretty_json "$req_body"
        echo
    fi
    echo "response:"
    pretty_json "$resp_body"
    echo
}

request() {
    local expected="$1"
    local method="$2"
    local url="$3"
    local req_body="${4-}"

    echo "=== ${method} ${url} ==="
    if [[ "$VERBOSE" == "1" && -n "$req_body" ]]; then
        echo "request body:"
        pretty_json "$req_body"
        echo
    fi

    local body_file
    body_file="$(mktemp)"
    local status
    local curl_args=(-sS --connect-timeout 5
        -o "$body_file" -w "%{http_code}"
        -X "$method"
        -H "Token: ${TOKEN}"
        -H "Content-Type: application/json")
    if [[ -n "$req_body" ]]; then
        curl_args+=(-d "$req_body")
    fi
    set +e
    status="$(curl "${curl_args[@]}" "$url")"
    local curl_rc=$?
    set -e
    local resp_body
    resp_body="$(cat "$body_file")"
    rm -f "$body_file"

    if [[ "$curl_rc" -ne 0 ]]; then
        echo "error: curl failed talking to ${url} (is the server running?)" >&2
        exit 1
    fi

    echo "status: ${status}"
    if [[ "$VERBOSE" == "1" ]]; then
        echo "response:"
        pretty_json "$resp_body"
    fi
    echo

    if [[ "$status" != "$expected" ]]; then
        echo "FAIL: expected HTTP ${expected}, got ${status}" >&2
        if [[ "$VERBOSE" != "1" ]]; then
            dump_bodies "$req_body" "$resp_body" >&2
        fi
        exit 1
    fi

    LAST_BODY="$resp_body"
}

TOKEN=""
STAMP="smoke-$(date +%s)-$$"
EMAIL="${STAMP}@example.com"
LOGIN_ID="${STAMP}"
LAST_BODY=""

echo "BASE_URL=${BASE_URL}"
echo
echo "Seeding token via POST /auth/login."
echo

LOGIN_BODY='{"User":{"LoginId":"admin","Password":"admin1234"}}'
request 200 POST "${BASE_URL}/auth/login" "$LOGIN_BODY"
TOKEN="$(extract_json_str "$LAST_BODY" token)"
if [[ -z "${TOKEN}" ]]; then
    echo "error: could not read token from login response" >&2
    exit 1
fi
echo "Token=${TOKEN}"
echo

echo "Person CRUD happy path."
echo

CREATE_BODY=$(printf '{"personName":"Smoke Test","personEmail":"%s","personAddress":"1 Smoke St"}' "$EMAIL")
request 201 POST "${BASE_URL}/person" "$CREATE_BODY"

PERSON_ID="$(extract_json_int "$LAST_BODY" personId)"
if [[ -z "${PERSON_ID}" ]]; then
    echo "error: could not read personId from create response" >&2
    exit 1
fi
echo "created personId=${PERSON_ID}"
echo

request 200 GET "${BASE_URL}/person"
request 200 GET "${BASE_URL}/person/${PERSON_ID}"

UPDATE_BODY=$(printf '{"personName":"Smoke Test Updated","personEmail":"%s","personAddress":null}' "$EMAIL")
request 200 PUT "${BASE_URL}/person/${PERSON_ID}" "$UPDATE_BODY"

echo "Person CRUD error paths."
echo

BAD_PERSON_BODY=$(printf '{"personEmail":"%s"}' "$EMAIL")
request 400 POST "${BASE_URL}/person" "$BAD_PERSON_BODY"
request 409 POST "${BASE_URL}/person" "$CREATE_BODY"

request 200 DELETE "${BASE_URL}/person/${PERSON_ID}"
request 404 GET "${BASE_URL}/person/${PERSON_ID}"

echo "User CRUD happy path."
echo

request 200 GET "${BASE_URL}/users"

CREATE_USER_BODY=$(printf '{"usrsFirstName":"Smoke","usrsLastName":"Test","usrsLoginId":"%s","usrsLoginPass":"smoke1234"}' "$LOGIN_ID")
request 201 POST "${BASE_URL}/users" "$CREATE_USER_BODY"

USER_ID="$(extract_json_int "$LAST_BODY" usrsId)"
if [[ -z "${USER_ID}" ]]; then
    echo "error: could not read usrsId from create response" >&2
    exit 1
fi
echo "created usrsId=${USER_ID}"
echo

request 200 GET "${BASE_URL}/users/${USER_ID}"

UPDATE_USER_BODY=$(printf '{"usrsFirstName":"Smoke","usrsLastName":"Updated","usrsLoginId":"%s","usrsLoginPass":"smoke5678"}' "$LOGIN_ID")
request 200 PUT "${BASE_URL}/users/${USER_ID}" "$UPDATE_USER_BODY"

echo "User CRUD error paths."
echo

request 400 POST "${BASE_URL}/users" '{"usrsLoginPass":"x"}'
request 409 POST "${BASE_URL}/users" "$CREATE_USER_BODY"

request 200 DELETE "${BASE_URL}/users/${USER_ID}"
request 404 GET "${BASE_URL}/users/${USER_ID}"

echo "Auth logout, then gated resource with the same token."
echo

request 200 POST "${BASE_URL}/auth/logout"
request 401 GET "${BASE_URL}/person"

echo "api-smoketest: ok"
