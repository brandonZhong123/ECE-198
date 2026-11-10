#!/bin/bash
cd -- "$(dirname -- "$0")" || exit 1
export PATH="/opt/homebrew/bin:/usr/local/bin:$PATH"

if command -v python3 >/dev/null; then
    python3 SaveReadings.py "$@"
    status=$?
else
    echo "Python 3 is required to run the logger."
    status=1
fi

if [[ $status -ne 0 && -t 0 ]]; then
    read -r -p "Press Return to close. "
fi
exit "$status"
