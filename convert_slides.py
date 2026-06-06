import re
import sys
import os

def convert_md_to_remark(md_text: str) -> str:
    # 3. Convert mermaid code fences (MANDATORY - before splitting on ---)
    def replace_mermaid(m):
        return f'.mermaid[\n<pre>\n{m.group(1).strip()}\n</pre>\n]'
    text = re.sub(r'```mermaid\n(.*?)```', replace_mermaid, md_text, flags=re.DOTALL)

    # 7. Split into slides
    slides = [s.strip() for s in re.split(r'\n---\n', text) if s.strip()]

    cleaned = []
    for slide in slides:
        slide = re.sub(r'^---\n', '', slide)
        slide = re.sub(r'\n---$', '', slide)
        cleaned.append(slide)
    slides = cleaned

    # 8. Annotate slides
    annotated = []
    for i, slide in enumerate(slides):
        first_line = slide.split('\n')[0].strip()
        is_qa = 'Q&A' in first_line
        is_end = 'End of Presentation' in first_line or 'Thank you' in first_line
        is_section = slide.count('\n') <= 2 and slide.startswith('## ')
        if i == 0:
            annotated.append(f'count: false\nclass: nord-dark, middle, center\n\n{slide}')
        elif is_qa or is_end or i == len(slides) - 1:
            annotated.append(f'count: false\nclass: nord-dark, middle, center\n\n{slide}')
        elif is_section:
            annotated.append(f'class: nord-light, middle, center\n\n{slide}')
        else:
            annotated.append(slide)

    # 9. Prepend layout directive
    result = 'layout: true\nclass: typo, typo-selection\n\n---\n' + '\n\n---\n\n'.join(annotated)
    return result

def build_html(remark_content: str, title: str) -> str:
    return f'''<!doctype html>
<html>
  <head>
    <title>{title}</title>
    <meta charset="utf-8" />
    <meta name="viewport"
      content="user-scalable=no,initial-scale=1,maximum-scale=1,minimum-scale=1,width=device-width" />
    <link rel="stylesheet" type="text/css" href="../katex/katex.min.css" />
    <link rel="stylesheet" type="text/css" href="../css/spaces.css" />
    <link rel="stylesheet" type="text/css" href="../css/slides.css" />
    <link rel="stylesheet" type="text/css" href="../css/nord-dark.css" />
    <link rel="stylesheet" type="text/css" href="../css/nord-light.css" />
    <link rel="stylesheet" type="text/css" href="../css/font-nord.css" />
    <link rel="stylesheet" type="text/css" href="../css/bg-nord.css" />
    <link rel="stylesheet" type="text/css" href="../css/style.css" />
  </head>
  <body>
    <textarea id="source">
{remark_content}
    </textarea>
    <script src="../js/remark.min.js"></script>
    <script src="../katex/katex.min.js" type="text/javascript"></script>
    <script src="../katex/contrib/auto-render.min.js" type="text/javascript"></script>
    <script src="../js/mermaid.min.js"></script>
    <script type="text/javascript">
      renderMathInElement(document.getElementById('source'), {{
        delimiters: [
          {{ left: '$$', right: '$$', display: true }},
          {{ left: '$', right: '$', display: false }},
        ],
      }});
      var slideshow = remark.create({{
        ratio: '16:10',
        highlightStyle: 'tomorrow-night-blue',
        highlightLines: true,
        countIncrementalSlides: false,
        navigation: {{
          scroll: false,
          touch: true,
          click: false,
        }},
      }});
    </script>
    <script src="../js/mermaid-init.js"></script>
  </body>
</html>'''

def main():
    # Read the markdown file
    input_path = sys.argv[1]
    output_path = sys.argv[2]
    
    with open(input_path, 'r', encoding='utf-8') as f:
        md_text = f.read()
    
    # Convert
    remark_content = convert_md_to_remark(md_text)
    html = build_html(remark_content, "Hypergraph Partitioning Showdown")
    
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(html)
    
    print(f"Written to {output_path}")

if __name__ == '__main__':
    main()
