#!/bin/bash


if [ -e .env ]; then
    eval "$(cat .env <(echo) <(declare -x))"
    [ -z "$STUDENT_NAME" ] && echo "WARN: STUDENT_NAME is empty"
    [ -z "$STUDENT_ID" ] && echo "WARN: STUDENT_NAME is empty"
    [ -z "$STUDENT_UTID" ] && echo "WARN: STUDENT_UTID is empty"
else
    STUDENT_NAME="筑波太郎"
    STUDENT_ID="202311XXX"
    STUDENT_UTID="s2311XXX"
fi


for file in $(find . -type f -name 'report*.raw.txt'); do
    directory=$(dirname $file)
    output=$(echo $file | sed "s/.raw//")
    # 出力ファイルを空にする
    > "$output"
    while IFS= read -r line; do
        if [[ "$line" =~ ^__INCLUDE\(([^\)]+)\)__ ]]; then
            include_file="$directory/${BASH_REMATCH[1]}"
            # include_file の内容を出力ファイルに追加
            if [[ -f "$include_file" ]]; then
                cat "$include_file" >> "$output"
            else
                echo "ERROR: $include_file not found."
                echo "ERROR: $include_file not found." >> "$output"
            fi
        else
            echo "$line" |\
                sed -e "s/__STUDENT_NAME__/$STUDENT_NAME/g" \
                -e "s/__STUDENT_ID__/$STUDENT_ID/g" \
                -e "s/__STUDENT_UTID__/$STUDENT_UTID/g" \
                >> "$output"
        fi
    done < "$file"
done
