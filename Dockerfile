FROM fedora:38 AS build
RUN dnf install -y cpprest-devel
RUN dnf install -y g++
RUN dnf install -y cmake
RUN dnf install -y zlib-devel zlib-static


WORKDIR /space-traders

COPY src/ ./src/
COPY CMakeLists.txt .

# compile
WORKDIR /space-traders/build
RUN cmake -DCMAKE_BUILD_TYPE=Release ..
RUN cmake --build .

RUN groupadd testg 
RUN useradd testu -g testg
USER testu
ENTRYPOINT [ "src/space-traders" ]


# multi-stage build, however, the libraries are not copied over so need to reinstall and cost a lot of time
# FROM fedora:38
# RUN dnf install -y cpprest-devel

# RUN groupadd testg 
# RUN useradd testu -g testg
# USER testu

# COPY --chown=testu:testg --from=build /space-traders/build/src/space-traders ./app/
# ENTRYPOINT [ "./app/space-traders"]