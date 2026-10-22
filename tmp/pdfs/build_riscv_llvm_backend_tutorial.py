#!/usr/bin/env python3
"""Build the RISC-V LLVM backend tutorial PDF from its controlled Markdown."""

from __future__ import annotations

import argparse
import html
import re
import textwrap
from pathlib import Path
from typing import Iterable

from reportlab.lib import colors
from reportlab.lib.enums import TA_CENTER, TA_LEFT
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle, getSampleStyleSheet
from reportlab.lib.units import mm
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.platypus import (
    CondPageBreak,
    HRFlowable,
    ListFlowable,
    ListItem,
    PageBreak,
    Paragraph,
    SimpleDocTemplate,
    Spacer,
    Table,
    TableStyle,
    XPreformatted,
)


REGULAR_FONT = "/usr/local/share/fonts/truetype/noto/NotoSansSC-Regular.ttf"
BOLD_FONT = "/usr/local/share/fonts/truetype/noto/NotoSansSC-Bold.ttf"
MONO_FONT = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
CORE_CHAPTERS = {
    "先建立后端地图",
    "案例一：整数加法如何变成 add",
    "案例二：函数调用与栈帧",
    "案例三：RVV 与 vsetvli",
    "形成自己的源码阅读方法",
}


def register_fonts() -> None:
    required = (REGULAR_FONT, BOLD_FONT, MONO_FONT)
    missing = [path for path in required if not Path(path).is_file()]
    if missing:
        raise FileNotFoundError(f"required fonts are missing: {', '.join(missing)}")
    if "NotoCJK" not in pdfmetrics.getRegisteredFontNames():
        pdfmetrics.registerFont(TTFont("NotoCJK", REGULAR_FONT))
        pdfmetrics.registerFont(TTFont("NotoCJKBold", BOLD_FONT))
        pdfmetrics.registerFont(TTFont("CodeMono", MONO_FONT))
        pdfmetrics.registerFontFamily(
            "NotoCJK", normal="NotoCJK", bold="NotoCJKBold",
            italic="NotoCJK", boldItalic="NotoCJKBold"
        )


def inline_markup(text: str) -> str:
    """Convert the small inline Markdown subset accepted by the tutorial."""
    escaped = html.escape(text, quote=False)
    escaped = re.sub(
        r"`([^`]+)`",
        lambda match: f'<font name="CodeMono" size="8.1">{match.group(1)}</font>',
        escaped,
    )
    escaped = re.sub(r"\*\*([^*]+)\*\*", r"<b>\1</b>", escaped)
    escaped = re.sub(r"\[([^]]+)]\(([^)]+)\)", r'<a href="\2" color="#1E5AA8">\1</a>', escaped)
    return escaped


def make_styles() -> dict[str, ParagraphStyle]:
    base = getSampleStyleSheet()
    return {
        "title": ParagraphStyle(
            "TutorialTitle", parent=base["Title"], fontName="NotoCJKBold",
            fontSize=25, leading=34, textColor=colors.HexColor("#17365D"),
            alignment=TA_CENTER, spaceBefore=25 * mm, spaceAfter=10 * mm,
        ),
        "h2": ParagraphStyle(
            "H2", parent=base["Heading1"], fontName="NotoCJKBold",
            fontSize=17, leading=23, textColor=colors.HexColor("#17365D"),
            spaceBefore=8 * mm, spaceAfter=4 * mm, keepWithNext=True,
        ),
        "h3": ParagraphStyle(
            "H3", parent=base["Heading2"], fontName="NotoCJKBold",
            fontSize=12.5, leading=18, textColor=colors.HexColor("#2F5597"),
            spaceBefore=5 * mm, spaceAfter=2.2 * mm, keepWithNext=True,
        ),
        "body": ParagraphStyle(
            "BodyCJK", parent=base["BodyText"], fontName="NotoCJK",
            fontSize=9.4, leading=15.8, textColor=colors.HexColor("#222222"),
            alignment=TA_LEFT, spaceAfter=2.8 * mm, wordWrap="CJK",
        ),
        "quote": ParagraphStyle(
            "Quote", parent=base["BodyText"], fontName="NotoCJK",
            fontSize=9, leading=15, textColor=colors.HexColor("#26415E"),
            leftIndent=2 * mm, rightIndent=2 * mm, wordWrap="CJK",
        ),
        "list": ParagraphStyle(
            "ListCJK", parent=base["BodyText"], fontName="NotoCJK",
            fontSize=9, leading=14, textColor=colors.HexColor("#222222"),
            spaceAfter=0.4 * mm, wordWrap="CJK",
        ),
        "code": ParagraphStyle(
            "Code", parent=base["Code"], fontName="CodeMono",
            fontSize=7.2, leading=10.4, textColor=colors.HexColor("#1F2933"),
            leftIndent=2.5 * mm, rightIndent=2.5 * mm, topPadding=2.5 * mm,
            bottomPadding=2.5 * mm, spaceBefore=1.5 * mm, spaceAfter=3 * mm,
            borderColor=colors.HexColor("#D7DEE8"), borderWidth=0.5,
            borderPadding=2.5 * mm, backColor=colors.HexColor("#F5F7FA"),
            splitLongWords=True,
        ),
        "table": ParagraphStyle(
            "TableCell", parent=base["BodyText"], fontName="NotoCJK",
            fontSize=7.7, leading=11.5, textColor=colors.HexColor("#222222"),
            wordWrap="CJK",
        ),
        "table_head": ParagraphStyle(
            "TableHead", parent=base["BodyText"], fontName="NotoCJKBold",
            fontSize=7.8, leading=11.5, textColor=colors.white, wordWrap="CJK",
        ),
        "footer": ParagraphStyle(
            "Footer", parent=base["BodyText"], fontName="NotoCJK",
            fontSize=7.5, leading=9, textColor=colors.HexColor("#687386"),
        ),
    }


def wrap_code(code: str, width: int = 95) -> str:
    wrapped: list[str] = []
    for raw in code.splitlines() or [""]:
        if len(raw) <= width:
            wrapped.append(raw)
            continue
        indent = re.match(r"\s*", raw).group(0)
        wrapped.extend(
            textwrap.wrap(
                raw, width=width, subsequent_indent=indent + "  ",
                replace_whitespace=False, drop_whitespace=False,
                break_long_words=True, break_on_hyphens=False,
            ) or [raw]
        )
    return "\n".join(wrapped)


def code_markup(code: str) -> str:
    """Keep ASCII code monospaced while rendering CJK runs with a CJK font."""
    escaped = html.escape(code)
    return re.sub(
        r"([\u2E80-\u9FFF\u3000-\u303F\uFF00-\uFFEF]+)",
        r'<font name="NotoCJK">\1</font>',
        escaped,
    )


def parse_table(lines: list[str], styles: dict[str, ParagraphStyle], width: float) -> Table:
    rows = [[cell.strip() for cell in line.strip().strip("|").split("|")] for line in lines]
    if len(rows) > 1 and all(re.fullmatch(r":?-{3,}:?", cell) for cell in rows[1]):
        rows.pop(1)
    columns = max(len(row) for row in rows)
    for row in rows:
        row.extend([""] * (columns - len(row)))
    formatted = []
    for row_index, row in enumerate(rows):
        style = styles["table_head"] if row_index == 0 else styles["table"]
        formatted.append([Paragraph(inline_markup(cell), style) for cell in row])
    table = Table(formatted, colWidths=[width / columns] * columns, repeatRows=1, hAlign="LEFT")
    table.setStyle(TableStyle([
        ("BACKGROUND", (0, 0), (-1, 0), colors.HexColor("#2F5597")),
        ("VALIGN", (0, 0), (-1, -1), "TOP"),
        ("GRID", (0, 0), (-1, -1), 0.35, colors.HexColor("#AEB9C8")),
        ("ROWBACKGROUNDS", (0, 1), (-1, -1), [colors.white, colors.HexColor("#F5F7FA")]),
        ("LEFTPADDING", (0, 0), (-1, -1), 5),
        ("RIGHTPADDING", (0, 0), (-1, -1), 5),
        ("TOPPADDING", (0, 0), (-1, -1), 4),
        ("BOTTOMPADDING", (0, 0), (-1, -1), 4),
    ]))
    return table


def markdown_flowables(markdown: str, styles: dict[str, ParagraphStyle], width: float) -> list:
    lines = markdown.splitlines()
    story: list = []
    paragraph: list[str] = []
    index = 0
    seen_core_chapter = False

    def flush_paragraph() -> None:
        if paragraph:
            story.append(Paragraph(inline_markup(" ".join(part.strip() for part in paragraph)), styles["body"]))
            paragraph.clear()

    while index < len(lines):
        line = lines[index]
        stripped = line.strip()
        if stripped.startswith("```"):
            flush_paragraph()
            index += 1
            code: list[str] = []
            while index < len(lines) and not lines[index].strip().startswith("```"):
                code.append(lines[index])
                index += 1
            story.append(XPreformatted(code_markup(wrap_code("\n".join(code))), styles["code"]))
            index += 1
            continue
        if stripped.startswith("# "):
            flush_paragraph()
            story.append(Paragraph(inline_markup(stripped[2:]), styles["title"]))
            story.append(HRFlowable(width="62%", thickness=1.2, color=colors.HexColor("#7D9BC1"), spaceAfter=8 * mm))
            index += 1
            continue
        if stripped.startswith("## "):
            flush_paragraph()
            title = stripped[3:]
            if title in CORE_CHAPTERS:
                if seen_core_chapter:
                    story.append(CondPageBreak(65 * mm))
                elif story:
                    story.append(PageBreak())
                seen_core_chapter = True
            story.append(Paragraph(inline_markup(title), styles["h2"]))
            index += 1
            continue
        if stripped.startswith("### "):
            flush_paragraph()
            story.append(Paragraph(inline_markup(stripped[4:]), styles["h3"]))
            index += 1
            continue
        if stripped.startswith("> "):
            flush_paragraph()
            quote = Paragraph(inline_markup(stripped[2:]), styles["quote"])
            box = Table([[quote]], colWidths=[width], hAlign="LEFT")
            box.setStyle(TableStyle([
                ("BACKGROUND", (0, 0), (-1, -1), colors.HexColor("#EAF1F8")),
                ("BOX", (0, 0), (-1, -1), 0.5, colors.HexColor("#A9BED7")),
                ("LINEBEFORE", (0, 0), (0, -1), 3, colors.HexColor("#2F75B5")),
                ("LEFTPADDING", (0, 0), (-1, -1), 9),
                ("RIGHTPADDING", (0, 0), (-1, -1), 9),
                ("TOPPADDING", (0, 0), (-1, -1), 7),
                ("BOTTOMPADDING", (0, 0), (-1, -1), 7),
            ]))
            story.extend([box, Spacer(1, 3 * mm)])
            index += 1
            continue
        if stripped.startswith("|") and stripped.endswith("|"):
            flush_paragraph()
            table_lines: list[str] = []
            while index < len(lines) and lines[index].strip().startswith("|"):
                table_lines.append(lines[index])
                index += 1
            story.extend([parse_table(table_lines, styles, width), Spacer(1, 3 * mm)])
            continue
        list_match = re.match(r"^\s*(-|\d+\.)\s+(.+)$", line)
        if list_match:
            flush_paragraph()
            ordered = list_match.group(1) != "-"
            items = []
            while index < len(lines):
                match = re.match(r"^\s*(-|\d+\.)\s+(.+)$", lines[index])
                if not match or (match.group(1) != "-") != ordered:
                    break
                items.append(ListItem(Paragraph(inline_markup(match.group(2)), styles["list"])))
                index += 1
            list_options = {
                "bulletType": "1" if ordered else "bullet",
                "leftIndent": 7 * mm,
                "bulletFontName": "NotoCJK",
                "bulletFontSize": 8,
                "spaceAfter": 2.4 * mm,
            }
            if ordered:
                list_options["start"] = "1"
            else:
                list_options["start"] = "-"
            story.append(ListFlowable(items, **list_options))
            continue
        if not stripped:
            flush_paragraph()
        elif stripped == "---":
            flush_paragraph()
            story.append(HRFlowable(width="100%", thickness=0.5, color=colors.HexColor("#C8D1DC"), spaceAfter=3 * mm))
        else:
            paragraph.append(line)
        index += 1
    flush_paragraph()
    return story


def page_decorations(canvas, doc) -> None:
    canvas.saveState()
    page_width, page_height = A4
    canvas.setStrokeColor(colors.HexColor("#D5DCE5"))
    canvas.setLineWidth(0.4)
    canvas.line(doc.leftMargin, page_height - 13 * mm, page_width - doc.rightMargin, page_height - 13 * mm)
    canvas.setFont("NotoCJK", 7.2)
    canvas.setFillColor(colors.HexColor("#687386"))
    canvas.drawString(doc.leftMargin, page_height - 10 * mm, "RISC-V LLVM 后端源码阅读教程")
    canvas.drawRightString(page_width - doc.rightMargin, 9 * mm, f"第 {doc.page} 页")
    canvas.restoreState()


def build_pdf(markdown_path: str, output_path: str) -> None:
    """Build an A4, searchable PDF from the tutorial's Markdown subset."""
    register_fonts()
    source = Path(markdown_path)
    destination = Path(output_path)
    if not source.is_file():
        raise FileNotFoundError(f"Markdown source does not exist: {source}")
    destination.parent.mkdir(parents=True, exist_ok=True)
    doc = SimpleDocTemplate(
        str(destination), pagesize=A4,
        leftMargin=19 * mm, rightMargin=19 * mm,
        topMargin=18 * mm, bottomMargin=16 * mm,
        title="RISC-V LLVM 后端源码阅读教程",
        author="MIC-for-learning",
        subject="LLVM RISC-V backend source reading",
    )
    styles = make_styles()
    story = markdown_flowables(source.read_text(encoding="utf-8"), styles, doc.width)
    doc.build(story, onFirstPage=page_decorations, onLaterPages=page_decorations)


def main(argv: Iterable[str] | None = None) -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("markdown_path")
    parser.add_argument("output_path")
    args = parser.parse_args(argv)
    build_pdf(args.markdown_path, args.output_path)


if __name__ == "__main__":
    main()
