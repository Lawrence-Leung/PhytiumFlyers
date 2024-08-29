#!/bin/sh

git update-index --chmod=+x ./install.sh
git update-index --chmod=+x ./*.sh
git update-index --chmod=+x ./scripts/*.sh
git update-index --chmod=+x ./make/*.mk
git update-index --chmod=+x ./standalone/lib/Kconfiglib/*.py