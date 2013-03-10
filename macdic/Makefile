###########################

# You need to edit these values.

#DICT_NAME		=	"英辞郎あ"
DICT_NAME		=	"英辞郎"
#DICT_NAME		=	"英辞郎な"
#DICT_NAME		=	"英辞郎た"
#DICT_NAME		=	"apple"

DICT_BUILD = bin/build_dict.sh
DICT_BUILD_OPTS =
# Suppress adding supplementary key.
# DICT_BUILD_OPTS		=	-s 0	# Suppress adding supplementary key.

###########################

DICT_DEV_KIT_OBJ_DIR	=	./objects
export	DICT_DEV_KIT_OBJ_DIR

DESTINATION_FOLDER	=	~/Library/Dictionaries
RM			=	/bin/rm

#EIJIRO_XMLS = EIJI-135.xml WAEI-135.xml RYAKU135.xml REIJI135.xml
EIJIRO_XMLS = EIJI-136.xml WAEI-136.xml RYAKU136.xml REIJI136.xml
EIJIRO_ATAG_XMLS = EIJI-136.xml WAEI-136-atag.xml RYAKU136.xml REIJI136-atag.xml
#EIJIRO_XMLS = EIJI-135.xml waei-136-atag.xml RYAKU135.xml reiji136-atag.xml

###########################


all: Eijiro.xml
	$(DICT_BUILD) $(DICT_BUILD_OPTS) $(DICT_NAME) Eijiro.xml Eijiro.css Eijiro.plist
	echo "Done."

check: jing.out

jing.out: Eijiro.xml
	java -jar jing/bin/jing.jar ~/work/ddk/documents/DictionarySchema/AppleDictionarySchema.rng Eijiro.xml &> jing.out

prepare: Eijiro.xml

#Eijiro.xml: TEXT.xml $(EIJIRO_XMLS) ALL-pre.xml ALL-post.xml
#	cat ALL-pre.xml $(EIJIRO_XMLS) TEXT.xml ALL-post.xml > Eijiro.xml

Eijiro.xml: TEXT.xml $(EIJIRO_ATAG_XMLS) ALL-pre.xml ALL-post.xml
	cat ALL-pre.xml $(EIJIRO_ATAG_XMLS) TEXT.xml ALL-post.xml > Eijiro.xml

reiji135-atag.xml: reiji135-tag.xml atag.sed #atag.awk
	sed -f atag.sed reiji135-tag.xml > reiji135-atag.xml
#	awk -f atag.awk reiji135-tag.xml | sed -f atag.sed > reiji135-atag.xml

#reiji136-atag.xml: reiji136-tag.xml atag.sed atag.awk
#	awk -f atag.awk reiji136-tag.xml | sed -f atag.sed > reiji136-atag.xml
#
#waei-136-atag.xml: waei-136-tag.xml atag.sed atag.awk
#	awk -f atag.awk waei-136-tag.xml | sed -f atag.sed > waei-136-atag.xml

REIJI136-atag.xml: REIJI136.xml atag.sed atag.awk
	sed -f atag.sed REIJI136.xml > REIJI136-atag.xml
#	awk -f atag.awk REIJI136.xml | sed -f atag.sed > REIJI136-atag.xml

WAEI-136-atag.xml: WAEI-136.xml atag.sed atag.awk
	sed -f atag.sed WAEI-136.xml > WAEI-136-atag.xml

TEXT.xml: tjoin.sh
	rm -f TEXT.xml
	find TEXT/ -type f -name \*.HTM -exec sh tjoin.sh {} >> TEXT.xml \;
	find TEXT/ -type f -name \*.TXT -exec sh tjoin.sh {} >> TEXT.xml \;


install:
	echo "Installing into $(DESTINATION_FOLDER)".
	mkdir -p $(DESTINATION_FOLDER)
	ditto --noextattr --norsrc $(DICT_DEV_KIT_OBJ_DIR)/$(DICT_NAME).dictionary  $(DESTINATION_FOLDER)/$(DICT_NAME).dictionary
	touch $(DESTINATION_FOLDER)
	echo "Done."
	echo "To test the new dictionary, try Dictionary.app."

clean:
	$(RM) -rf $(DICT_DEV_KIT_OBJ_DIR)
