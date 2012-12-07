BEGIN {
  FS = OFS = "\t"

  entries = name ".entries"
  surfaces = name ".surfaces"
  definitions = name ".definitions"
  aliases = name ".aliases"

  system("rm -f " entries)
  system("rm -f " surfaces)
  system("rm -f " definitions)
  system("rm -f " aliases)
}

/^entries/ {
  print $2, $3, $4, $5 >> entries
  next
}

/^surfaces/ {
  gsub(/^ */, "", $2)
  gsub(/ *$/, "", $2)
  print $2, $3 >> surfaces
  next
}

/^definitions/ {
  gsub(/<a href=[^>]*>/, "", $6)
  gsub(/<\/a>/, "", $6)
  gsub(/[ \t\n]*$/, "", $6);
  print $2, $3, $4, $5, $6 >> definitions
  next
}

/^aliases/ {
  print $2, $3, $4 >> aliases
  next
}
