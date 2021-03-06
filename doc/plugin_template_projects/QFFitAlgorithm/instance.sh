#! /bin/bash

function replace_in_all {
	echo "replace: " $1 
	for i in ./$DIRNAME/*.cpp ./$DIRNAME/*.h ./$DIRNAME/*.pro ./$DIRNAME/*.qrc ./$DIRNAME/*.ui ./$DIRNAME/help/*.html ./$FNAME/help/*.ini; do 
	  echo "  -> " $i
	  sed $1 $i > $i.temp 
	  mv $i.temp $i
	done
}

CLASSNAME=$1
TARGETNAME=$2
FITALGCCLASSNAME=$3
FITALGCNAME=$4
BASENAME=fitalgorithm_inst

echo "This script will create a set of usable files from this template. 
In this course you will be asked for some information (e.g. class names ...)!

"

if [ "$CLASSNAME" == "" ]; then
	echo -n "class name for QFFitAlgorithm plugin implementation: "
	read -e CLASSNAME
else
	echo -n "class name for QFFitAlgorithm plugin implementation: " $CLASSNAME "
"
fi

if [ "$TARGETNAME" == "" ]; then
	echo -n "name for plugin library (also plugin ID): "
	read -e TARGETNAME
else
	echo -n "name for plugin library (also plugin ID): " $TARGETNAME "
"
fi


if [ "$FITALGCCLASSNAME" == "" ]; then
	echo -n "class name for first fit algorithm: "
	read -e FITALGCCLASSNAME
else
	echo -n "class name for first fit algorithm: " $FITALGCCLASSNAME "
"
fi


if [ "$FITALGCNAME" == "" ]; then
	echo -n "ID for first fit algorithm: "
	read -e FITALGCNAME
else
	echo -n "ID for first fit algorithm: " $FITALGCNAME "
"
fi

CLASSNAME_UC=`echo $CLASSNAME | tr a-z A-Z`
CLASSNAMEFA_UC=`echo ${FITALGCCLASSNAME} | tr a-z A-Z`

FNAME=`echo $CLASSNAME | tr A-Z a-z`
echo -n "file name for QFFitAlgorithm plugin implementation: " $FNAME ".*
"
FNAMEFF=`echo $FITALGCCLASSNAME | tr A-Z a-z`
echo -n "file name for QFFitAlgorithm implementation: " $FNAMEFF ".*
"

DIRNAME=fitalgorithm_${TARGETNAME}
mkdir -p $DIRNAME

for i in ${BASENAME}.*; do 
  cp -v "$i" "./$DIRNAME/${FNAME}${i/$BASENAME}"; 
done

for i in ${BASENAME}_a1*.*; do 
  cp -v "$i" "./$DIRNAME/${FNAMEFF}${i/${BASENAME}_a1}"; 
done

mkdir -p $DIRNAME/translations
mkdir -p $DIRNAME/help
mkdir -p $DIRNAME/help/pic
cp ftarget.html $DIRNAME/help/${TARGETNAME}.html
cp f1.html $DIRNAME/help/${FITALGCNAME}.html
cp tutorial.html $DIRNAME/help/tutorial.html
cp tutorials.ini $DIRNAME/help/tutorials.ini



replace_in_all 's/QFFitAlgorithmInst_A1/'$FITALGCCLASSNAME'/g'
replace_in_all 's/QFFitAlgorithmInst/'$CLASSNAME'/g'
replace_in_all 's/target_id/'$TARGETNAME'/g'
replace_in_all 's/fa_id/'$FITALGCNAME'/g'

replace_in_all 's/'$BASENAME'_a1config.h/'$FNAMEFF'config.h/g'
replace_in_all 's/'$BASENAME'_a1config.cpp/'$FNAMEFF'config.cpp/g'
replace_in_all 's/'$BASENAME'_a1config.ui/'$FNAMEFF'config.ui/g'
replace_in_all 's/'$BASENAME'_a1.h/'$FNAMEFF'.h/g'
replace_in_all 's/'$BASENAME'_a1.cpp/'$FNAMEFF'.cpp/g'
replace_in_all 's/HEADER_A1_H/'$CLASSNAMEFA_UC'_H/g'
replace_in_all 's/HEADER_A1_CONFIG_H/'$CLASSNAMEFA_UC'_CONFIG_H/g'

replace_in_all 's/'$BASENAME'.png/'$FNAME'.png/g'
replace_in_all 's/'$BASENAME'.h/'$FNAME'.h/g'
replace_in_all 's/'$BASENAME'.cpp/'$FNAME'.cpp/g'
replace_in_all 's/'$BASENAME'.qrc/'$FNAME'.qrc/g'
replace_in_all 's/'$BASENAME'.ui/'$FNAME'.ui/g'
replace_in_all 's/HEADER_H/'$CLASSNAME_UC'_H/g'

replace_in_all 's/doxygen_GROUPNAME/qf3fitfunp_'$TARGETNAME'/g'
replace_in_all 's/a1.html/'$FITALGCNAME'.html/g'

for i in ${DIRNAME}/*.pro; do 
  mv -v "$i" "${DIRNAME}/${DIRNAME}.pro"; 
done