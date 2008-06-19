include("common.qs");

var textBlockFormat = new QTextBlockFormat.clone(defaultBlockFormat);

cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is a simple test message for paragraph content test case");
cursor.insertBlock(defaultBlockFormat);
cursor.insertText("This is a new paragraph, different from the previous line\nbut these two lines are in the same\n paragraph as the second line of this document.");

cursor.insertBlock(defaultBlockFormat);

cursor.insertBlock(defaultBlockFormat);
cursor.insertText("All white space should be collapsed.");

cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);
// cursor.insertText("\n");

cursor.insertBlock(defaultBlockFormat);
cursor.insertText("There's space before and after this paragraph, but all paragraph leading and trailing spaces should be stripped.");

cursor.insertBlock(defaultBlockFormat);

cursor.insertBlock(defaultBlockFormat);
cursor.insertText("Ten          spaces with <text:s text:c=\"9\" /> with one default space.");

cursor.insertBlock(defaultBlockFormat);
cursor.insertText("Ten          spaces with <text:s text:c=\"10\" />.");

cursor.insertBlock(defaultBlockFormat);
cursor.insertText(" Three   spaces with <text:s /> and one each at the start and end ");

cursor.insertBlock(defaultBlockFormat);
cursor.insertText("\n");

cursor.insertBlock(defaultBlockFormat);
cursor.insertText("One\ttab with <text:tab text:tab-ref=\"3\" /> to hint its size only. No worries for a layouting word processor.");

cursor.insertBlock(defaultBlockFormat);
cursor.insertText("Three\t\t\ttabs with <text:tab/>.");

cursor.insertBlock(defaultBlockFormat);
cursor.insertText("\n");

cursor.insertBlock(defaultBlockFormat);
cursor.insertText("Three line breaks\n\n\n with <text:line-break/> end here.");

cursor.insertBlock(defaultBlockFormat);

cursor.insertBlock(defaultBlockFormat);
cursor.insertText("To check the following features, you should try moving the last words to the edge of a laid out line by typing or deleting text from the line \n");

cursor.insertBlock(defaultBlockFormat);
cursor.insertText("Soft Hyphen: The last word has a minus sign after \"recrea\" but the last but one word has a soft-hyphen at the same location: recrea­tion recrea-tion");

cursor.insertBlock(defaultBlockFormat);
cursor.insertText("Non-breaking Hyphen: The last word has a minus sign after \"re\" but the last but one word has a non-breaking hyphen at the same location: re‑creation re-creation");

cursor.insertBlock(defaultBlockFormat);
cursor.insertText("No break space: The last word set has a simple space but the last but one word set has a no-break space: \"austin powers\" \"austin powers\"");

return document;
