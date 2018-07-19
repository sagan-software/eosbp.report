#!/bin/sh
WORK_DIR=/repo/eosbp.report
GIT_DIR=/repo/eosbp.report.git
BUILD_TREE=build
BUILD_WORK_DIR=$WORK_DIR/$BUILD_TREE
BUILD_GIT_DIR=$GIT_DIR/worktrees/$BUILD_TREE

cd $GIT_DIR
rm -Rf $BUILD_WORK_DIR
git worktree remove $BUILD_TREE
git worktree add $BUILD_WORK_DIR gh-pages

cd $WORK_DIR
node ./scripts/fetch_json.bundle.js

git --git-dir=$BUILD_GIT_DIR --work-tree=$BUILD_WORK_DIR add .
git --git-dir=$BUILD_GIT_DIR --work-tree=$BUILD_WORK_DIR commit -am "$(date --iso-8601=minutes --utc)"
git --git-dir=$BUILD_GIT_DIR --work-tree=$BUILD_WORK_DIR push origin gh-pages
