#/bin/bash
echo "PACKAGING RELEASE FOR CSOUNDVST3"
echo "First use Xcode to clean, build, archive, and make distrbution for the 'CsoundVST - All' scheme."
echo "Then change to the distribution directory and run this script in it."

# Validate the artifacts.
echo
echo "CsoundVST.vst3 VST3 plugin"
echo
codesign --verify --deep --strict --display --verbose=2 "Products/Users/michaelgogins/Library/Audio/Plug-Ins/VST3/CsoundVST3.vst3"
echo
echo "CsoundVST.component AudioUnit plugin"
echo
codesign --verify --deep --strict --display --verbose=2 "Products/Users/michaelgogins/Library/Audio/Plug-Ins/Components/CsoundVST3.component"
echo
echo "CsoundVST.app standalone app"
echo
codesign --verify --deep --strict --display --verbose=2 "Products/Applications/CsoundVST3.app"
echo

# Create an archive containing all artifacts.
cd Products
zip -rq CsoundVST3-Release.zip ./*

echo "Release package:"
echo
ls -ll
echo
unzip -l CsoundVST3-Release.zip