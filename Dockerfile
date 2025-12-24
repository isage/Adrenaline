FROM vitasdk/vitasdk:latest

RUN wget https://github.com/pspdev/pspdev/releases/latest/download/pspdev-ubuntu-latest-x86_64.tar.gz

RUN mkdir -p /usr/local/pspdev
RUN tar -xzf pspdev-ubuntu-latest-x86_64.tar.gz -C /usr/local/pspdev

ENV PSPDEV /usr/local/pspdev
ENV PATH ${PSPDEV}/bin:$PATH

# COPY --from=1 --chown=user ${PSPDEV} ${PSPDEV}