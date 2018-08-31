### Outset Range Demo

This demo shows off a minimal UI with a functioning text demo:

The board boots up to the starting view: `SPLASH_STATE`. You can press the trackpad button to continue.

The next view is the `TEXT_HISTORY_STATE`. Here you can review the messages you've received and sent, a history of your conversation with another Outset node. You can press the green button on the keyboard to start writing a message.

After you've pressed the green button, the next view is the `TEXT_MESSAGE_STATE`. Here you can type directly from the keyboard to compile your message. To write in uppercase, hold down the shift-like button (looks like an up arrow with `aA`) while simultaneously pressing a character key. To write a symbol, hold down the `sym` button while simultaneously pressing a character key.

Once you're happy with the message, press the blackberry key (looks like 8 dots next to the trackpad) to send the message. You should be taken directly back to `TEXT_HISTORY_STATE` once your message has been sent.

#### Install Dependencies
There should be some empty libraries in the `/lib` directory. You must run the 2 commands to initialize your local config file (see `.gitmodules`) and then to fetch all lib repos.
```
$ git submodule init
$ git submodule update
```
