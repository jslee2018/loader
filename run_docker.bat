FOR /F %%I IN ("%0") DO SET BATDIR=%%~dpI
docker run --name ubuntu -it -v %BATDIR%:/home/workspace --security-opt seccomp=unconfined ubuntu