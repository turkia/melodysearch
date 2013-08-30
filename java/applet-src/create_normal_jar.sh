#!/bin/sh
#
# The following files need to be writeable:
#	Duke.x509
#	signedWriteFile.jar
#
# Also, you need to have keytool, jar, and jarsigner in your path.

PATH=/bin:/usr/local/j2sdk1.3.1/bin/:$PATH

KEYSTORE=keystore.cbrahms
STOREPASS=rfeva34j

KEYTOOL_NOT_FOUND=`which keytool | grep "^no" | wc -l`
JAR_NOT_FOUND=`which jar | grep "^no" | wc -l`
JARSIGNER_NOT_FOUND=`which jarsigner | grep "^no" | wc -l`

ANY_NOT_FOUND=`expr $KEYTOOL_NOT_FOUND + $JAR_NOT_FOUND + $JARSIGNER_NOT_FOUND`

if [ $ANY_NOT_FOUND -ne 0 ]; then
	echo "$ANY_NOT_FOUND required tools not found."
	exit
fi


# Step 1: Create the archive. 

echo " "
echo "Create the archive:"
javac MidiSynth.java AlgorithmSearchApplet.java
jar cfv AlgorithmSearchApplet.jar *.class

# Step 2: Sign the archive. 

echo " "
echo "Sign the archive:"
jarsigner -verbose -keystore $KEYSTORE -storepass $STOREPASS AlgorithmSearchApplet.jar CBrahms

# Step 3: Show the contents of the signed archive. 

echo " "
echo "Contents of the archive are: "
jar tvf AlgorithmSearchApplet.jar

# Step 4: Show the keystore contents. 

echo " "
echo "Contents of keystore are: "
keytool -list -keystore $KEYSTORE -storepass $STOREPASS

# Step 5: Clean up!

cp AlgorithmSearchApplet.jar ../html
rm *.class

