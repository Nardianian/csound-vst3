#/bin/bash
PROJECT_DIR="~/csound-vst3"
BUILD_DIR="$PROJECT_DIR/Builds/MacOSX/build/Release"
OUTPUT_DIR="$PROJECT_DIR/Artifacts"

# Build all targets in the project for release.
cd "$PROJECT_DIR"
xcodebuild -project Builds/MacOSX/CsoundVST3.xcodeproj -scheme All -configuration Release

# Marshal the artifacts.
mkdir -p "$OUTPUT_DIR"
cp -R "$BUILD_DIR/CsoundVST3.vst3" "$OUTPUT_DIR/"
cp -R "$BUILD_DIR/CsoundVST3.component" "$OUTPUT_DIR/"
cp -R "$BUILD_DIR/CsoundVST3.app" "$OUTPUT_DIR/"

# Validate the artifacts.
codesign --verify --deep --strict --display --verbose=2 "$OUTPUT_DIR/CsoundVST3.vst3"
codesign --verify --deep --strict --display --verbose=2 "$OUTPUT_DIR/CsoundVST3.component"
codesign --verify --deep --strict --display --verbose=2 "$OUTPUT_DIR/CsoundVST3.app"

# Create an archive containing all artifacts.
cd "$OUTPUT_DIR"
zip -r CsoundVST3-Release.zip ./*

echo "Archive created at $OUTPUT_DIR/CsoundVST3-Release.zip"