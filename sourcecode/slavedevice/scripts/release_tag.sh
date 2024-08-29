#!/bin/sh

version=v0.0.3
branch=master
git update-index --chmod=+x ./install.sh
git update-index --chmod=+x ./*.sh
git update-index --chmod=+x ./scripts/*.sh
git update-index --chmod=+x ./make/*.mk
git add .
git tag -a $version -m "Release $version"
git commit -m "Release $version"
git push origin $branch --tags