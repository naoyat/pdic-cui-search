BEGIN {
  print ".mode tabs"
  print ".import " name ".entries entries"
  print ".import " name ".surfaces surfaces"
  print ".import " name ".definitions definitions"
  print ".import " name ".aliases aliases"
  exit;
}
