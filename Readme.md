# Boost http server

Credit to:   
[Simple-Web-Server](https://gitlab.com/eidheim/Simple-Web-Server.git)  
[Simple-WebSocket-Server](https://gitlab.com/eidheim/Simple-WebSocket-Server.git)  

# What did I do

Intergrate the two libs above mentioned, and distill some simple api for:

1. restful api
2. serve static dir files(for spa web)
3. websocket server
4. cron routine job, like repeatedly broadcast ws msg
5. async asio loop with multi threads with optional tls server

P.S. please refer to app.cpp/cli.cpp in examples dir

# Dependence

- [boost](https://github.com/boostorg/boost)
- [openssl](https://github.com/openssl/openssl)
- [lua](https://github.com/lua/lua)
- [sol2](https://github.com/ThePhD/sol2)


