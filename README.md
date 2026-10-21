# Quicklog

a small logging helper written in qt quick qml for my obsidian vault

<img width="644" height="160" alt="QuickLog 0.2.0" src="docs/quicklog-0.2.0.png" />

the initial idea:
<img width="1027" height="601" alt="image" src="https://github.com/user-attachments/assets/818b3894-20e0-432e-bdba-9b0763ed801f" />

## macos

```sh
brew install qtbase qtdeclarative cmake ninja
make install
```

this builds `QuickLog.app` into `~/Applications` with qt bundled inside. it opens in the bottom right corner of the screen under the mouse and floats over full screen apps

to open it with ctrl+\`:

1. make a new shortcut in the shortcuts app
2. add the "open app" action and pick quicklog
3. open the shortcut's details, click "add keyboard shortcut" and press ctrl+\`

`make format`, `make lint` and the pre-commit hooks also need `brew install clang-format llvm pre-commit`
