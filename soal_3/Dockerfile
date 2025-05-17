
FROM ubuntu:20.04

RUN apt-get update && \
	DEBIAN_FRONTEND=noninteractive apt-get install -y \
	build-essential gcc fuse3 libfuse3-dev pkg-config

COPY antink.c /antink.c
RUN gcc /antink.c -o /antink `pkg-config fuse3 --cflags --libs`

CMD ["/antink", "/home/antink_mount"]

