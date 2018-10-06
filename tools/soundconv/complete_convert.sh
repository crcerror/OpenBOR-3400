#!/usr/bin/env bash
DIR=$(dirname "$(readlink -f "$0")")
TARGET=$1
if [[ -e "$TARGET/music" ]] ; then
	$DIR/convertsound.pl "$TARGET"
	if [[ "$?" == "0" ]] ; then
		$DIR/convertsound.pl "$TARGET" 1
		$DIR/convertsoundlinks.pl "$TARGET"
	fi
else
	echo "$TARGET/music not found, seems no valid data-dir!"
fi
