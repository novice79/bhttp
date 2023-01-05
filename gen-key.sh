#!/usr/bin/env bash
	
# brew install openssl@1.1
# brew install openssl@3
# brew info openssl
# sudo ln -sf /usr/local/opt/openssl@3/bin/openssl /usr/local/bin/openssl

mkdir -p misc
# openssl req -new -newkey rsa:2048 -days 3650 -nodes -x509 \
# -subj "/C=CN/ST=HuNan/L=ChangSha/O=py/CN=localhost" \
# -addext 'subjectAltName = "DNS:foo.co.uk,DNS:127.0.0.1,DNS:example.com,DNS:example.net"' \
# -keyout misc/key.pem -out misc/cert.pem

openssl req -x509 -newkey rsa:2048 -sha256 -days 3650 -nodes \
  -keyout misc/server.key -out misc/server.crt -subj '/CN=example.com' \
  -addext 'subjectAltName=DNS:example.com,DNS:example.net'