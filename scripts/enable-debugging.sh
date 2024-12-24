#!/bin/sh

if [ -z "$1" ]; then
	echo "Usage: $0 <binary>"
	exit 1
fi

entitlement="$(mktemp)"
cat >"$entitlement" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>com.apple.security.get-task-allow</key>
	<true/>
</dict>
</plist>
EOF

if ! codesign -s - --entitlements "$entitlement" -f "$1"; then
	rm -f "$entitlement"
	exit 1
else
	rm -f "$entitlement"
fi
