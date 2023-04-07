npm create vite@latest spa -- --template react
<!-- before 164K    dist/bin/www -->
npm i lodash jotai uuid buffer -S
npm install @mui/material @emotion/react @emotion/styled @mui/icons-material @fontsource/roboto -S
<!-- after 732K    dist/bin/www -->
<!-- merge from branch lua. some git commands memo -->
<!-- do not use checkout from another branch, cause it will auto staged
git checkout lua -- bhttp/client.hpp
git checkout lua -- bhttp/util.hpp
git checkout lua -- examples
git checkout lua -- Readme.md 
-->
<!-- unstage all: git reset -->

<!-- get file from another branch without stage it -->
git restore --source lua -- build.sh
<!-- or -->
git show lua:bhttp/client_wss.hpp > bhttp/client_wss.hpp
git show lua:bhttp/client_https.hpp > bhttp/client_https.hpp
git diff ..lua -- bhttp/client_https.hpp

<!-- show node file size -->
echo $(($(wc -c $(which node) | awk '{ print $1 }')/1024/1024))M
ll $(realpath `which node`)