FROM gabrieleara/dev_environment:cpp

# Note: rpm is necessary to build the corresponding package,
# but it cannot be properly built on Ubuntu

RUN export DEBIAN_FRONTEND=noninteractive \
    && apt-get update \
    && apt-get -y install \
    libyaml-dev \
    devscripts \
    && apt-get -y autoremove \
    && apt-get -y clean \
    && rm -rf /var/lib/apt/lists/*
