#!/bin/sh
WORK_DIR=/repo/eosbp.report
GIT_DIR=/repo/eosbp.report.git

cd $GIT_DIR
rm -Rf $WORK_DIR/build
git worktree add $WORK_DIR/build gh-pages

cd $WORK_DIR
yarn install
yarn build-prod
node ./scripts/fetch_json.js
