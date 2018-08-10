### Outset Range Demo

This demo shows off a minimal UI with two things happening:

`MATTE` is listening for messages<br>
`GREY` is sending messages

Usually, we would attach `GREY` to a static location or give it to someone to hold, while another party walks around holding `MATTE`. It's currently programmed to auto send messages and replies so there's no need for human input.

#### Install Dependencies
There should be some empty libraries in the `/lib` directory. You must run the 2 commands to initialize your local config file (see `.gitmodules`) and then to fetch all lib repos.
```
$ git submodule init
$ git submodule update
```

#### Install Instructions
Compile `outsetDemoClient.cpp` and `outsetDemoServer.cpp` separately. One outset gets flashed with the client version and one outset gets flashed with the server version.

#### Notes
When `GREY` sends a message out and then immediately waits for a reply... that reply (whether `GREY` receives an acknowledgement or not always gets met with the same output... for now.
