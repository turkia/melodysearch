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

# Step 1: Create identity with new keypair and self-signed certificate.

echo " "
echo "Create identity "CBrahms" with new keypair and self-signed certificate:"
keytool -genkey -alias 'CBrahms' -dname \
	"cn=C-Brahms Group, ou=MelodySearch, o=University of Helsinki, c=fi" \
	-keystore $KEYSTORE -storepass $STOREPASS -keypass $STOREPASS \
	-validity 1000

# Step 1a: Export certificate

echo " "
echo "Export certificate"
keytool -export -alias 'CBrahms' -rfc -file CBrahms.x509 \
	-keystore $KEYSTORE -storepass $STOREPASS

# Step 2: Show the keystore contents. 

echo " "
echo "Contents of keystore are: "
keytool -list -keystore $KEYSTORE -storepass $STOREPASS

# Step 6: Clean up!

rm -f *.class

