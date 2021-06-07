FROM gabrieleara/dev_environment:cpp

# RPM is necessary to build the corresponding package

RUN export DEBIAN_FRONTEND=noninteractive \
    && apt-get update \
    && apt-get -y install rpm \
    && apt-get -y autoremove \
    && apt-get -y clean \
    && rm -rf /var/lib/apt/lists/*
