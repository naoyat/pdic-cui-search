#!/bin/sh
#
# TXT -> d:entry
#
FILENAME=$1

nkf -w $FILENAME | awk -v filename="$FILENAME" 'BEGIN {
  type = (filename ~ /.HTM/) ? "htm" : "txt";
  gsub(/^.*TEXT\/+/, "", filename)
  printf("[%s] %s\n", type, filename) > "/dev/stderr";
  printf("<d:entry id=\"f.%s\" d:title=\"file:%s\">\n", filename, filename)
  printf("\t<d:index d:value=\"file:%s\" />\n", filename)
  if (type == "txt") print "<p>"
  p_said = 0;
}

/^<html/ { next }
/^<head/ { next }
/^<meta/ { next }
/^<STYLE/ { next }
/^<\/STYLE/ { next }
/^<title/ { gsub(/title/, "h1"); print; print "<p>"; p_said++; next }
/^<\/head/ { next }
/^<BODY/ { next }
/^<\/body/ { next }
/^<\/html/ { next }

{
  if (type == "txt") {
    gsub(/[\n\r]/, "")
    gsub(/&/, "\\&amp;")
    gsub(/</, "\\&lt;")
    gsub(/>/, "\\&gt;")
  } else {
    gsub(/<font face=\"Verdana, Comic Sans MS\">/, "<span class=\"text\">")
    gsub(/<\/font>/, "</span>")
    gsub(/<font\/>/, "</span>")
    gsub(/<br>/, "<br />")
    gsub(/<hr>/, "<hr />")
    # gsub(/<hr>/, "--------<br />")
    gsub(/<img [^>]*/, "& /")
    gsub(/<IMG [^>]*/, "& /")
    gsub(/<IMG /, "<img ")
    gsub(/<img border="0" /, "<img ")
    gsub(/\.gif" \/>/, ".gif\" alt=\"・\" />")
    gsub(/height="18" \/>/, "height=\"18\" alt=\"・\" />")
    gsub(/height="32" \/>/, "height=\"32\" alt=\"・\" />")
    gsub(/<</, "<") # 大抵typo
    if (!p_said) { print "<p>"; p_said++; }
  }

  if (type == "txt") {
    print $0 "<br />"
#   print $0
  } else {
    print $0
  }
}

END {
  if (type == "txt") print "</p>"
  else print "</p>"
  printf("</d:entry>\n")
  printf("\n")
}
'
