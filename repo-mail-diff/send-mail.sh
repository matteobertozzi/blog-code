#!/bin/bash

diff_dir="~/.repo-diffs"
mail_address="th30z@localhost"

for html_file in `ls -1 $diff_dir/*.html` ; do
    repo_name=`basename $html_file | sed 's/\.html$//g'`
    diff_file=`echo $html_file | sed 's/\.html$/\.diff/g'`

    boundary="mail-boundary-`echo $repo_name | md5sum | cut -d ' ' -f -1`"
    alt_boundary="mail-alt-boundary-`echo $boundary | md5sum | cut -d ' ' -f -1`"

    echo "Send Repo Diff $repo_name - $html_file"
    (
        echo "MIME-Version: 1.0"
        echo "Subject: Repo-Diff: $repo_name"
        echo "To: $mail_address"
        echo "Content-Type: multipart/mixed; boundary=$boundary"

        echo "--$boundary"
        echo "Content-Type: multipart/alternative; boundary=$alt_boundary"
        echo

        echo "--$alt_boundary"
        echo "Content-Type: text/plain"
        echo
        cat $diff_file

        echo "--$alt_boundary"
        echo "Content-Type: text/html"
        echo
        cat $html_file

        echo
        echo "--$alt_boundary--"
        echo "--$boundary"
        echo "Content-Type: Application/Binary_Attachment; name=\"`basename $diff_file`\""
        echo "Content-Disposition: attachment; filename=\"`basename $diff_file`\""
        echo "Content-Transfer-Encoding: uuencode"
        echo
        uuencode $diff_file $diff_file
    ) | sendmail $mail_address

    if [ $? -eq 0 ]; then
        echo "    [ ok ] $repo_name"
        rm $html_file
        rm $diff_file
    else
        echo "    [ !! ] $repo_name"
    fi
done

