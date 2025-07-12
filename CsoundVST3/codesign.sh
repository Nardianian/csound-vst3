#/bin/bash
# Use your actual signing identity here
IDENTITY="Apple Development: Michael Gogins (JC94KD4B5T)"

# VST3
codesign --deep --force --options runtime --sign "$IDENTITY" \
    build/CsoundVST3_artefacts/VST3/CsoundVST3.vst3

# AU
codesign --deep --force --options runtime --sign "$IDENTITY" \
    build/CsoundVST3_artefacts/AU/CsoundVST3.component

# Standalone
codesign --deep --force --options runtime --sign "$IDENTITY" \
    build/CsoundVST3_artefacts/Standalone/CsoundVST3.app
