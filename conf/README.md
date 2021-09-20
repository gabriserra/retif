# Retif configuration

> **NOTE**: This section differs from our published papers. Be careful.

Configuring the Retif daemon requires modifying the `retifconf.yaml` file, which is typically installed alongside the other components of the framework in `/etc/retifconf.yaml`.

> This document is still under revision, but the
> [retifconf.yaml](retifconf.yaml) file present in this directory is well
> documented with a lot of comments, so please check that out for now.

Refer to the default installed configuration on your system during Retif
installation process for more information about how to configure the Retif
daemon.

The configuration can be changed while the daemon is running. However, it will
not take effect until the daemon is restarted. This can be achieved by simply
using
```sh
sudo service retif restart
```
<!-- TODO: is this true? -->
> **NOTICE**: All running applications managed by the Retif daemon will be
> reverted to normal (non-realtime) priority when the daemon is stopped. Make
> sure that relevant applications are not running before stopping or restarting
> the daemon!
