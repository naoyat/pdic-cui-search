/<(d:entry id=|d:index d:value=)"/ {
  gsub(/&lt;a href=&quot.*&quot; target=&quot;_parent&quot;&gt;/, "")
  gsub(/&lt;\/a&gt;/, "")
  print
  next
}
{
  print
}

