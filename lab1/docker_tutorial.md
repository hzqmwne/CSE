<!DOCTYPE HTML>
<html>
 <head>
  <meta charset="utf-8"/>
  <link href="http://cdnjs.cloudflare.com/ajax/libs/highlight.js/8.1/styles/github.min.css" rel="stylesheet"/>
 </head>
 <body>
  <h1 id="a-brief-introduction-of-docker">
   Docker Tutorial
  </h1>
  <p>
   If you want to learn other commands not included in this tutorial or other things about docker, you can always check in
   <a href="https://docs.docker.com/">
    Docker’s Doc
   </a>
   .
  </p>
  <h2 id="whats-containerdocker">
   What’s Container(Docker)
  </h2>
  <p>
   An
   <strong>
    image
   </strong>
   is a lightweight, stand-alone, executable package that includes everything needed to run a piece of software, including the code, a runtime, libraries, environment variables, and config files.
  </p>
  <p>
   A
   <strong>
    container
   </strong>
   is a runtime instance of an image—what the image becomes in memory when actually executed. It runs completely isolated from the host environment by default, only accessing host files and ports if configured to do so.
  </p>
  <p>
   Containers run apps natively on the host machine’s kernel. They have better performance characteristics than virtual machines that only get virtual access to host resources through a hypervisor. Containers can get native access, each one running in a discrete process, taking no more memory than any other executable.
  </p>
  <h2 id="install-docker">
   Install Docker
  </h2>
  <p>
   Even though Docker can be installed by Linux, Mac, and Windows. We strongly recommend that you
   <strong>
    install Docker in a Linux system
   </strong>
   (Ubuntu or Debian).
  </p>
  <h3 id="install-docker-in-ubuntu">
   Install Docker in ubuntu
  </h3>
  <p>
   Following the instructions in the:
   <a href="https://docs.docker.com/engine/installation/linux/docker-ce/ubuntu/">
    Get Docker CE for Ubuntu
   </a>
  </p>
  <h3 id="install-docker-in-debian">
   Install Docker in Debian
  </h3>
  <p>
   Following the instructions in the:
   <a href="https://docs.docker.com/engine/installation/linux/docker-ce/debian/">
    Get Docker CE for Debian
   </a>
  </p>
  <h3 id="test-docker">
   Test docker
  </h3>
  <p>
   When you finish the installation, you can test whether the docker is successfully installed by
  </p>
  <blockquote>
   <p>
    sudo docker run hello-world
   </p>
  </blockquote>
  <p>
   You will see some results like
  </p>
  <blockquote>
   <p>
    Hello from Docker!
    <br/>
    This message shows that your installation appears to be working correctly.
    <br/>
    ....
   </p>
  </blockquote>
  <p>
   which means your docker is now successfully installed.
  </p>
  <h2 id="examples">
   Examples
  </h2>
  <h3 id="hello-world">
   hello world
  </h3>
  <p>
   type the command:
  </p>
  <blockquote>
   <p>
    $ sudo docker run hello-world
   </p>
  </blockquote>
  <p>
   you will see some results like
  </p>
  <blockquote>
   <p>
    Hello from Docker!
    <br/>
    This message shows that your installation appears to be working correctly.
    <br/>
    ....
   </p>
  </blockquote>
  <h3 id="list-images">
   List images
  </h3>
  <blockquote>
   <p>
    $ sudo docker images
   </p>
  </blockquote>
  <p>
   will list all the docker image in the current system
  </p>
  <h3 id="pull-new-docker-images">
   Pull new docker images
  </h3>
  <p>
   You can search a docker image in the
   <a href="https://hub.docker.com/">
    DockerHub
   </a>
  </p>
  <p>
   When you find an image you want to use, take the
   <code>
    cselab_env
   </code>
   <a href="https://hub.docker.com/r/ddnirvana/cselab_env/">
    image
   </a>
   as an example, you can pull the image by
  </p>
  <blockquote>
   <p>
    $ sudo  docker pull ddnirvana/cselab_env:latest
   </p>
  </blockquote>
  <p>
   here，
   <code>
    ddnirvana/sharekernel_env
   </code>
   is the name of the image,
   <code>
    :latest
   </code>
   is the tag of the image, which means the newest version of an image.
  </p>
  <p>
   After the
   <code>
    docker pull
   </code>
   success, you can see the image has been downloaded in your local machine by
   <code>
    docker images
   </code>
   command
  </p>
  <h3 id="run-an-image">
   Run an image
  </h3>
  <p>
   It’s very simple to run a docker image(start a container from an image)
  </p>
  <blockquote>
   <p>
    $ sudo docker run –rm -it ddnirvana/cse_lab:latest /bin/bash
   </p>
  </blockquote>
  <p>
   This command means we want to run a container from the image
   <code>
    ddnirvana/cselab_env:latest
   </code>
   .
   <code>
    -it
   </code>
   means attach the container’s terminal to current terminal.
   <code>
    --rm
   </code>
   means remove the container when you exit it.
   <code>
    /bin/bash
   </code>
   is the init program you want to run in the container.
  </p>
  <p>
   Now you will enter to a terminal from the container, and you can try some commands in the container. Every operations in the container is isolated from the outer system.
  </p>
  <p>
   type
  </p>
  <blockquote>
   <p>
    $ exit
   </p>
  </blockquote>
  <p>
   will leave the container
  </p>
  <p>
   You can learn more options about
   <code>
    docker run
   </code>
   by
  </p>
  <blockquote>
   <p>
    $ sudo docker run –help
   </p>
  </blockquote>
  <h3 id="run-a-exited-container">
   Run a exited Container
  </h3>
  <p>
   If you run a container without
   <code>
    --rm
   </code>
   options, actually you can re-enter the exited container.
  </p>
  <p>
   type
  </p>
  <blockquote>
   <p>
    $ sudo docker ps -a
   </p>
  </blockquote>
  <p>
   and you will see some results like
  </p>
  <blockquote>
   <p>
    CONTAINER ID        IMAGE                         COMMAND             CREATED             STATUS                     PORTS               NAMES
    <br/>
    8bec16084d97        test_commit:v1.0              “/bin/bash”         4 minutes ago       Exited (0) 4 minutes ago                       quirky_goldberg
   </p>
  </blockquote>
  <p>
   here, the
   <code>
    8bec16084d97
   </code>
   is your previous running container’s id.
  </p>
  <p>
   To re-enter this container, you should first start it, by
  </p>
  <blockquote>
   <p>
    $ sudo docker start 8bec16084d97
   </p>
  </blockquote>
  <p>
   replace
   <code>
    8bec16084d97
   </code>
   to
   <code>
    your container's id
   </code>
  </p>
  <p>
   and then, attach a terminal to the container, by
  </p>
  <blockquote>
   <p>
    $ sudo docker attach 8bec16084d97
   </p>
  </blockquote>
  <p>
   replace
   <code>
    8bec16084d97
   </code>
   to
   <code>
    your container's id
   </code>
   too
  </p>
  <p>
   Now, you will find you have re-enter the container and every things you have done are still here.
  </p>
  <h3 id="commit-a-container-to-an-image">
   Commit a container to an image
  </h3>
  <p>
   You can commit a container to an image anytime.
  </p>
  <p>
   First, get the container id by
  </p>
  <blockquote>
   <p>
    sudo docker ps -a
   </p>
  </blockquote>
  <p>
   and you will see some results like
  </p>
  <blockquote>
   <p>
    CONTAINER ID        IMAGE                         COMMAND             CREATED             STATUS                     PORTS               NAMES
    <br/>
    8bec16084d97        test_commit:v1.0              “/bin/bash”         4 minutes ago       Exited (0) 4 minutes ago                       quirky_goldberg
   </p>
  </blockquote>
  <p>
   and then, commit a container to an image using the container’s id
  </p>
  <blockquote>
   <p>
    sudo docker commit 8bec16084d97 test_commit:v1.0
   </p>
  </blockquote>
  <p>
   here,
   <code>
    8bec16084d97
   </code>
   should be replaced by your container’s id, and
   <code>
    test_commit
   </code>
   is the image name you want to give to your image, and
   <code>
    v1.0
   </code>
   is the tag name you want to give to your image.
  </p>
  <p>
   After that, you can see your new saved image by
  </p>
  <blockquote>
   <p>
    sudo docker images
   </p>
  </blockquote>
  <h3 id="run-cselab_env-image">
   Run CSELAB_ENV image
  </h3>
  <p>
   We have provided a docker image for your CSE Labs: ddnirvana/cselab_env:latest
  </p>
  <p>
   1 . Pull the docker image
  </p>
  <blockquote>
   <p>
    $ sudo docker pull ddnirvana/cselab_env:latest
   </p>
  </blockquote>
  <p>
   2 . create a directory to store Labs’s codes in the outer system, for example
  </p>
  <blockquote>
   <p>
    $ mkdir -p /home/cse/lab_hostdir
   </p>
  </blockquote>
  <p>
   3 . run a container
  </p>
  <blockquote>
   <p>
    $ sudo docker run -it  –privileged  –cap-add=ALL -v /home/cse/lab_hostdir/:/home/stu/devlop ddnirvana/cselab_env:latest /bin/bash
   </p>
  </blockquote>
  <p>
   This command means we want to run a container from the image
   <code>
    ddnirvana/cselab_env:latest
   </code>
   .
   <code>
    -it
   </code>
   means attach the container’s terminal to current terminal.
   <code>
    -v /home/cse/lab_hostdir/:/home/stu/devlop
   </code>
   means mount the host dir
   <code>
    /home/cse/lab_hostdir
   </code>
   to the container’s dir
   <code>
    /home/stu/devlop
   </code>
   .
  </p>
  <p>
   The default user in
   <code>
    ddnirvana/cselab_env:latest
   </code>
   is
   <code>
    stu
   </code>
   . The password for
   <code>
    stu
   </code>
   and
   <code>
    root
   </code>
   are both
   <code>
    000
   </code>
   .
  </p>
  <p>
   <strong>
    Notes
   </strong>
   : everything in a docker container will be cleaned after you exit a container, so you need to
   <mark>
    store every usefull files in a mounted volume
   </mark>
   , in the above example, only files in
   <code>
    /home/stu/devlop
   </code>
   in container will be persistent and every modification in these files will be persistent.
  </p>
  <h2 id="cse-labs">
   CSE Labs
  </h2>
  <h3 id="lab1">
   Lab1
  </h3>
  <p>
   In Lab1, after you pull cse Lab1s codes in some directory of the outer system(eg. /home/cse/lab_hostdir/)
  </p>
  <p>
   Then you could start a container for you Lab1 by
  </p>
  <blockquote>
   <p>
    $ sudo docker run -it  –privileged  –cap-add=ALL -v /home/cse/lab_hostdir/:/home/stu/devlop ddnirvana/cselab_env:latest /bin/bash
   </p>
  </blockquote>
  <p>
   You will find the Lab1’s codes appear in the
   <code>
    /home/stu/devlop
   </code>
   in the container. You can direclty write codes in the container or in the outer system. But you should run
   <code>
    make
   </code>
   and other grade scripts in the container.
  </p>
  <p>
   Here,
   <code>
    --privileged  --cap-add=ALL
   </code>
   will give the container ability to use fuse and do other privilege operations.
  </p>
 </body>
</html>


