#!/bin/bash

git checkout HEAD .
git clean -f
git reset
git cherry-pick --continue
