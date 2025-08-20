# cmake/EmbedText.cmake
# Usage:
#   embed_text_to_header(<INPUT> <OUT_HEADER> <SYMBOL_NAME>)
# Generates a header with `inline constexpr char SYMBOL_NAME[] = R"( ... )";`

function(embed_text_to_header INPUT OUT_HEADER SYMBOL)
  file(READ "${INPUT}" _content)
  # Optional: ensure newline at end for nicer diffs
  if(NOT _content MATCHES "\n$") 
    set(_content "${_content}\n")
  endif()
  # Use a custom delimiter to avoid )" collisions
  set(_delim EMBEDDED_CC)
  file(WRITE "${OUT_HEADER}" "/* auto-generated: ${INPUT} */\n#pragma once\n")
  file(APPEND "${OUT_HEADER}" "inline constexpr const char ${SYMBOL}[] = R\"${_delim}(\n${_content})${_delim}\";\n")
endfunction()
