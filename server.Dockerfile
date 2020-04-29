FROM gcc:8 AS builder

ARG BAM_SCRIPT_ARGUMENTS='static-libstdcpp=true'
ARG BAM_TARGET='server_release'

ADD . /code/
WORKDIR /code/

RUN mkdir -p bam && \
    cd bam && \
    wget https://github.com/matricks/bam/archive/v0.4.0.tar.gz -O bam.tar.gz && \
    tar -xzf bam.tar.gz -C . --strip-components=1 && \
    ./make_unix.sh && \
    cd .. && \
    ./bam/bam $BAM_SCRIPT_ARGUMENTS $BAM_TARGET


FROM frolvlad/alpine-glibc:latest

RUN addgroup -S tw_server && adduser -S -H tw_server -G tw_server && \
    mkdir -p "/app/data/mapres" && \
    echo 'add_path $DATADIR' > "/app/storage.cfg" && echo 'add_path $CURRENTDIR' >> "/app/storage.cfg" && \
    chown -R tw_server:tw_server "/app/"

COPY --from=builder --chown=tw_server:tw_server "/code/data/maps/" "/app/data/maps"
COPY --from=builder --chown=tw_server:tw_server "/code/db_srv" "/code/autoexec.cfg" "/code/entrypoint.sh" "/app/"

EXPOSE 8303/udp

ENTRYPOINT ["/app/entrypoint.sh"]
CMD ["./db_srv"]
