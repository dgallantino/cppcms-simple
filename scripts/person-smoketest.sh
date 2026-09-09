#!/usr/bin/env bash
set -euo pipefail

BASE_URL="${BASE_URL:-http://localhost:8080}"
BASE_URL="${BASE_URL%/}"

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

extract_token() {
    local body="$1"
    if command -v python3 >/dev/null 2>&1; then
        printf '%s' "$body" | python3 -c 'import json,sys; print(json.load(sys.stdin)["token"])'
    elif command -v python >/dev/null 2>&1; then
        printf '%s' "$body" | python -c 'import json,sys; print(json.load(sys.stdin)["token"])'
    else
        printf '%s' "$body" | sed -n 's/.*"token":"\([^"]*\)".*/\1/p' | head -n 1
    fi
}

extract_person_id() {
    local body="$1"
    if command -v python3 >/dev/null 2>&1; then
        printf '%s' "$body" | python3 -c 'import json,sys; print(int(json.load(sys.stdin)["personId"]))'
    elif command -v python >/dev/null 2>&1; then
        printf '%s' "$body" | python -c 'import json,sys; print(int(json.load(sys.stdin)["personId"]))'
    else
        printf '%s' "$body" | sed -n 's/.*"personId":\s*\([0-9][0-9]*\).*/\1/p' | head -n 1
    fi
}

request() {
    local expected="$1"
    local method="$2"
    local url="$3"
    local body="${4-}"

    echo "=== ${method} ${url} ==="
    if [[ -n "$body" ]]; then
        echo "request body:"
        pretty_json "$body"
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
    if [[ -n "$body" ]]; then
        curl_args+=(-d "$body")
    fi
    set +e
    status="$(curl "${curl_args[@]}" "$url")"
    local curl_rc=$?
    set -e
    local body
    body="$(cat "$body_file")"
    rm -f "$body_file"

    if [[ "$curl_rc" -ne 0 ]]; then
        echo "error: curl failed talking to ${url} (is the server running?)" >&2
        exit 1
    fi

    echo "status: ${status}"
    echo "response:"
    pretty_json "$body"
    echo

    if [[ "$status" != "$expected" ]]; then
        echo "FAIL: expected HTTP ${expected}, got ${status}" >&2
        exit 1
    fi

    LAST_BODY="$body"
}

TOKEN=""
EMAIL="smoke-$(date +%s)-$$@example.com"
LAST_BODY=""

echo "BASE_URL=${BASE_URL}"
echo
echo "Seeding token via POST /auth/login."
echo

LOGIN_BODY='{"User":{"LoginId":"admin","Password":"admin1234"}}'
request 200 POST "${BASE_URL}/auth/login" "$LOGIN_BODY"
TOKEN="$(extract_token "$LAST_BODY")"
if [[ -z "${TOKEN}" ]]; then
    echo "error: could not read token from login response" >&2
    exit 1
fi
echo "Token=${TOKEN}"
echo

CREATE_BODY=$(printf '{"personName":"Smoke Test","personEmail":"%s","personAddress":"1 Smoke St"}' "$EMAIL")
request 201 POST "${BASE_URL}/person" "$CREATE_BODY"

PERSON_ID="$(extract_person_id "$LAST_BODY")"
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

request 200 DELETE "${BASE_URL}/person/${PERSON_ID}"

echo "person-smoketest: ok"
