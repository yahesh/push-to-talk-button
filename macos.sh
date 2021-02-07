#!/bin/bash

VOLUME=$(osascript -e "input volume of (get volume settings)")

if [[ "$VOLUME" -eq 0 ]]
then
  osascript -e "set volume input volume 75"
else
  osascript -e "set volume input volume 0"
fi
