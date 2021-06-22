<!------------------------------ Project Shields ------------------------------>

<!-- NOTE:
---- See bottom of the page for reference links definitions
---->

[![Contributors][contributors-shield]][contributors-url]
[![Issues][issues-shield]][issues-url]
[![Bugs][bugs-shield]][bugs-url]
[![License][license-shield]][license-url]
[![Documentation][docs-shield]][docs-url]

<!-- [![Forks][forks-shield]][forks-url] -->
<!-- [![Stars][stars-shield]][stars-url] -->

# Retif - Real-Time Framework for POSIX Systems

Retif (or ReTiF) is a novel framework that enables unprivileged access to the
real-time features of Unix-based operating systems in a controlled manner. The
API it provides improves the usability of real-time capabilities of the
underlying OS with more guarantees than what one would normally get using
directly the OS API.

The main strength of the framework is its declarative approach, in which
applications simply declare the kind of tasks that they would like to run with
real-time privileges and let the framework do its "magic" to accomodate their
requests.

## Publications

The framework has been the subject of two publications from Scuola Superiore
Sant'Anna, Pisa:
 - Gabriele Serra, Gabriele Ara, Pietro Fara, and Tommaso Cucinotta (May 19-21,
   2020), "**[An Architecture for Declarative Real-Time Scheduling on Linux][paper-url]**".
   In *Proceedings of the 23rd IEEE International Symposium on Real-Time
   Distributed Computing* (IEEE ISORC 2020), Nashville, Tennessee, USA (pp.
   44-55), IEEE.
 - Gabriele Serra, Gabriele Ara, Pietro Fara, and Tommaso Cucinotta (*to
   appear*), "**ReTiF: A Declarative Real-Time Scheduling Framework for POSIX
   Systems**". In *Journal of Systems Architecture (JSA)*, Elsevier.

The presentation of the conference paper above can be found on
[YouTube][youtube-url]. Please refer to the journal paper above for a detailed
discussion on Retif internals. Feel free to contact any of the authors above for
a copy.

## Getting Started

As of now, Retif can be obtained by building its sources. Soon, with each
release of the framework we will attach packages for major linux distributions,
as well as self-contained binary archives.

### Prerequisites

Building Retif from sources requires a reasonably recent version of
[CMake][cmake-url] to be installed, which will be used to build all the
components of the framework and to install them in the appropriate locations.

### Installing from sources

1. Clone the repo
   ```sh
   git clone https://github.com/gabriserra/retif
   cd retif
   ```
2. Run the build script (it will invoke CMake for you)
   ```sh
   ./m build
   ```
3. Install (requires superuser privileges)
   ```sh
   sudo ./m install
   ```

## Usage

Applications that want to leverage the functionality provided by Retif must use
the *Retif library* (`libretif`) to do so, which provides a simple yet effective
declarative API for real-time applications. Following is an overview of Retif
ecosystem architecture.

![Retif architecture][retif-arch]

Retif is based on a *daemon-library* architecture, in which there is a centeral
decision authority, the *Retif daemon* (`retifd`), to which all requests from unprivileged
user applications are forwarded. The daemon performs a set of checks useful to
provide some real-time guarantees to user applications and it is the only
component in Retif that interacts with the underlying OS to set scheduling
properties of the real-time tasks declared by user applications.

The behavior of the daemon can be extended using a set of *plugins* which are
dynamically loaded by the daemon at startup according to a user-provided
configuration. The daemon is in fact implemented as much OS-independent and
scheduling algorithm-independent as possible, leaving all OS-specific and
scheduling algorithm-specific operations to plugins. Each plugin is typically
associated to a specific OS/scheduling algorithm (some plugins may support
multiple OSes as long as they share similar APIs, like POSIX).

For each request received by the daemon through the libretif API, the currently
loaded plygins are interrogated to analyze the request and determine whether
they can serve it (i.e. set the scheduling parameters of the requesting
application in order to satisfy its requirements) or not. If at least one plugin
can serve the request, the user application receives a positive response. Refer
to the [plugins](plugins) directory for more info.

### Running the Retif daemon

Retif relies on a daemon application to run in the background. To start the
daemon you can use `service` or `systemctl`:
```sh
sudo service retif start
```

You can check that the daemon is running by checking the exit code of the
`status` command, like this:
```sh
(
  service retif status &&
    echo 'Retif daemon is running'
) || echo 'Retif daemon is not running'
```
or alternatively you can check whether the `retifd` application is running
(works also if not started using the service command):
```sh
(
  pgrep retifd >/dev/null &&
    echo 'Retif daemon is running'
) || echo 'Retif daemon is not running'
```

### Configuring the Retif daemon

The set of plugins loaded by the daemon at startup depends on the *daemon
configuration file*, which is typically found in `/etc/retif.conf`. Refer to the
content of the [conf](conf) directory for more details.

### Retif API

The Retif library provides a simple API that streamlines communication with the
Retif daemon.

The following table shows the main functions exposed by the library:
| Function           | Description                                                                                                                                                                       |
| ------------------ | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `rtf_task_create`  | Performs task admission test and applies the specified `rtf_params` to the new task.                                                                                              |
| `rtf_task_change`  | Performs a new task admission test with the specified `rtf_params`; in case of failure the task maintains its old parameters.                                                     |
| `rtf_task_release` | Releases a task, freeing its resources and detaching the attached POSIX thread, if any.                                                                                           |
| `rtf_task_attach`  | Attaches a POSIX thread id to the given task.                                                                                                                                     |
| `rtf_task_detach`  | Detaches the POSIX thread assigned to a task; after this call, the thread runs with a non real-time priority and the task reference can then be attached to another POSIX thread. |

Applications can declare the scheduling parameters of each real-time task by
filling an instance of the opaque type `rtf_params`, using the functions
described in table below:

| Parameter             | Unit         | Getter / Setter                                             |
| --------------------- | ------------ | ----------------------------------------------------------- |
| Runtime               | microseconds | `rtf_params_get_runtime` / `rtf_params_set_runtime`         |
| Desired Runtime       | microseconds | `rtf_params_get_des_runtime` / `rtf_params_set_des_runtime` |
| Period                | microseconds | `rtf_params_get_period` / `rtf_params_set_period`           |
| Relative Deadline     | microseconds | `rtf_params_get_deadline` / `rtf_params_set_deadline`       |
| Priority              | -            | `rtf_params_get_priority` / `rtf_params_set_priority`       |
| Scheduling Plugin     | -            | `rtf_params_set_scheduler` / `rtf_params_get_scheduler`     |
| Ignore Admission Test | -            | `rtf_params_ignore_admission`                               |

For a more complete description of Retif library API, please refer to the
[online documentation][docs-url].

### Retif API Example

The following code provides a simple overview of how applications that leverage
Retif API are written.
```C
#include <retif.h>

void retif_thread()
{
    /* Task representation */
    struct rtf_task t = RTF_TASK_INIT;

    /* Task parameters */
    struct rtf_params p = RTF_PARAM_INIT;

    /* Connect to the daemon via a UNIX socket */
    if (rtf_connect() == RTF_CONNECTION_ERR)
        return; /* Unable to connect to the daemon. */

    /* Set task parameters */
    rtf_params_set_period(&p, T_PERIOD);
    rtf_params_set_runtime(&p, T_RUNTIME);
    rtf_params_set_des_runtime(&p, T_DES_RUNTIME);
    rtf_params_set_deadline(&p, T_DEADLINE);

    /* Test for admission */
    int res = rtf_task_create(&t, &p);

    /* If admission failed, we can retry with different
     * parameters */
    if (res == RTF_FAIL)
        return;
    else if (res == RTF_CONNECTION_ERR)
        return; /* Communication failed */

    /* res = RTF_OK */

    /* On success we attach an execution flow to the task
     * specification */
    rtf_task_attach(&t, getpid());

    /* Signals that a task begins its execution */
    rtf_task_start(&t);

    while (!computation_ended())
    {
        /* Task runs mandatory actions */
        mandatory_computation();

        /* Enabling optional computation depending on the
         * accepted runtime */
        if (rtf_task_get_accepted_runtime(&t) > T_RUNTIME)
            optional_computation();

        /* Suspend execution waiting for the next period */
        rtf_task_wait_period(&t);
    }

    /* Cleanup */
    rtf_task_release(&t);
}
```

## Roadmap

See the [open issues][issues-url] for a list of proposed features (and known
issues).

## Contributors

 - [**Gabriele Serra**][gabri-serra-url]
 - [**Gabriele Ara**][gabri-ara-url]
 - [**Pietro Fara**][pietro-fara-url]

The list of contributors can be found in the [contributors of the project][contributors-url].

> We want to thank [Prof. Tommaso Cucinotta][tommaso-cucinotta-url] for his
> extensive support to the work. Without him, the framework would not exist at
> all.

## License

The project comes with a GPLv3 license. If you want to use this code, you can do
without limitation but you have to document the modifications and include this
license. Read more [here](https://choosealicense.com/licenses/gpl-3.0/).

## Citation

If you want to cite, please refer to:

```bibtex
@inproceedings{serraetal:20:isorc,
    author =       {Serra, Gabriele and Ara, Gabriele and Fara, Pietro and Cucinotta, Tommaso},
    title =        {An Architecture for Declarative Real-TimeScheduling on Linux},
    year =         {2020}
    keywords =     {real-time, scheduling, declarative, linux},
}
```

> Journal article to appear soon.

<!-------------------------- Markdown Links & Images -------------------------->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->

<!------------------------ Images ------------------------->

[retif-arch]: img/retif-architecture.svg

<!------------------------- Urls -------------------------->

[paper-url]: https://doi.org/10.1109/ISORC49007.2020.00013
[youtube-url]: https://www.youtube.com/watch?v=9Y0KXTPXL14
[cmake-url]: https://cmake.org
[docs-url]: https://codedocs.xyz/gabriserra/retif

[bugs-url]: https://github.com/gabriserra/retif/labels/bug
[contributors-url]: https://github.com/gabriserra/retif/graphs/contributors
[forks-url]: https://github.com/gabriserra/retif/network/members
[issues-url]: https://github.com/gabriserra/retif/issues
[license-url]: https://github.com/gabriserra/retif/blob/master/LICENSE.txt
[stars-url]: https://github.com/gabriserra/retif/stargazers

[gabri-serra-url]: https://github.com/gabriserra
[gabri-ara-url]: https://github.com/gabrieleara
[pietro-fara-url]: https://github.com/pietrofara
[tommaso-cucinotta-url]: http://retis.sssup.it/~tommaso/eng/index.html

<!------------------------ Shields ------------------------>

[bugs-shield]: https://img.shields.io/github/issues/gabriserra/retif/bug
[contributors-shield]: https://img.shields.io/github/contributors/gabriserra/retif
[docs-shield]: https://codedocs.xyz/gabriserra/retif.svg
[forks-shield]: https://img.shields.io/github/forks/gabriserra/retif
[issues-shield]: https://img.shields.io/github/issues/gabriserra/retif
[license-shield]: https://img.shields.io/github/license/gabriserra/retif
[stars-shield]: https://img.shields.io/github/stars/gabriserra/retif
