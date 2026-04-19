FROM debian:bookworm-slim AS builder

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        binutils-mingw-w64-x86-64 \
        ca-certificates \
        cmake \
        g++-mingw-w64-x86-64-posix \
        ninja-build \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

COPY CMakeLists.txt README.md Start-Workbench.ps1 ./
COPY cmake ./cmake
COPY src ./src

RUN cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/workspace/cmake/toolchains/mingw-w64-x86_64.cmake

RUN cmake --build build

RUN mkdir -p /out \
    && cp build/WindowsSecurityWorkbench.exe /out/ \
    && cp build/WindowsSecurityWorkbenchGUI.exe /out/

FROM debian:bookworm-slim

WORKDIR /opt/wsw

COPY --from=builder /out ./dist

CMD ["sh", "-lc", "echo 'Cross-build completed. Windows artifacts:' && ls -lah /opt/wsw/dist"]
