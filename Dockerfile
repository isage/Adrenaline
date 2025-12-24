FROM vitasdk/vitasdk:latest

RUN wget https://github.com/pspdev/pspdev/releases/latest/download/pspdev-ubuntu-latest-x86_64.tar.gz

RUN mkdir -p /usr/local/pspdev
RUN tar -xzf pspdev-ubuntu-latest-x86_64.tar.gz -C /usr/local/

ENV PSPDEV=/usr/local/pspdev
ENV PATH=${PSPDEV}/bin:$PATH

RUN echo "export PSPDEV=${PSPDEV}" > /etc/profile.d/vitasdk.sh && \
    echo 'export PATH=${PSPDEV}/bin:$PATH'  >> /etc/profile.d/vitasdk.sh

# COPY --chown=user ${PSPDEV} ${PSPDEV}