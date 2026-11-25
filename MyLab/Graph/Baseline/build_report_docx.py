# -*- coding: utf-8 -*-
"""把 pandoc 生成的报告 docx 按中文排版要求重排。

- 中文宋体，英文/数字 Times New Roman
- 正文小四(12pt)，大标题小二(18pt)，次级三号(16pt)，再次级小三(15pt)
- 1.5 倍行距
- 表格五号、代码块等宽、图片居中、页脚页码
"""

import os
import sys

from docx import Document
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.oxml import OxmlElement
from docx.oxml.ns import qn
from docx.shared import Cm, Pt, RGBColor

SRC = sys.argv[1]
DST = sys.argv[2]

BODY_PT = 12.0        # 小四
TITLE_PT = 18.0       # 小二
SECTION_PT = 16.0     # 三号
SUBSECTION_PT = 15.0  # 小三
TABLE_PT = 10.5       # 五号
CAPTION_PT = 10.5
CODE_PT = 9.5

EAST = "宋体"
WEST = "Times New Roman"
MONO = "Consolas"


def set_run_fonts(run, east=EAST, west=WEST):
    run.font.name = west
    rpr = run._element.get_or_add_rPr()
    rfonts = rpr.find(qn("w:rFonts"))
    if rfonts is None:
        rfonts = OxmlElement("w:rFonts")
        rpr.append(rfonts)
    rfonts.set(qn("w:ascii"), west)
    rfonts.set(qn("w:hAnsi"), west)
    rfonts.set(qn("w:eastAsia"), east)
    rfonts.set(qn("w:cs"), west)


def style_paragraph_runs(paragraph, size, bold=False, mono=False):
    for run in paragraph.runs:
        east = EAST if mono else EAST
        west = MONO if mono else WEST
        set_run_fonts(run, east=east, west=west)
        if size is not None:
            run.font.size = Pt(size)
        if bold:
            run.font.bold = True
        if not mono:
            run.font.color.rgb = RGBColor(0, 0, 0)


def add_page_number(section):
    footer = section.footer
    paragraph = footer.paragraphs[0]
    paragraph.alignment = WD_ALIGN_PARAGRAPH.CENTER
    run = paragraph.add_run()
    set_run_fonts(run)
    run.font.size = Pt(TABLE_PT)
    begin = OxmlElement("w:fldChar")
    begin.set(qn("w:fldCharType"), "begin")
    instr = OxmlElement("w:instrText")
    instr.set(qn("xml:space"), "preserve")
    instr.text = "PAGE"
    end = OxmlElement("w:fldChar")
    end.set(qn("w:fldCharType"), "end")
    run._r.append(begin)
    run._r.append(instr)
    run._r.append(end)


def set_table_borders(table):
    tbl_pr = table._tbl.tblPr
    old = tbl_pr.find(qn("w:tblBorders"))
    if old is not None:
        tbl_pr.remove(old)
    borders = OxmlElement("w:tblBorders")
    for edge in ("top", "left", "bottom", "right", "insideH", "insideV"):
        element = OxmlElement("w:" + edge)
        element.set(qn("w:val"), "single")
        element.set(qn("w:sz"), "4")
        element.set(qn("w:space"), "0")
        element.set(qn("w:color"), "000000")
        borders.append(element)
    tbl_pr.append(borders)


def shade_cell(cell, color="F2F2F2"):
    tc_pr = cell._tc.get_or_add_tcPr()
    shd = OxmlElement("w:shd")
    shd.set(qn("w:val"), "clear")
    shd.set(qn("w:color"), "auto")
    shd.set(qn("w:fill"), color)
    tc_pr.append(shd)


def main():
    doc = Document(SRC)

    section = doc.sections[0]
    section.page_width = Cm(21.0)
    section.page_height = Cm(29.7)
    section.top_margin = Cm(2.54)
    section.bottom_margin = Cm(2.54)
    section.left_margin = Cm(2.54)
    section.right_margin = Cm(2.54)

    for paragraph in doc.paragraphs:
        style_name = paragraph.style.name
        fmt = paragraph.paragraph_format
        align = WD_ALIGN_PARAGRAPH.LEFT

        if style_name == "Heading 1":
            size, bold, spacing = TITLE_PT, True, 1.5
            align = WD_ALIGN_PARAGRAPH.LEFT
            fmt.space_before = Pt(0)
            fmt.space_after = Pt(18)
        elif style_name in ("Heading 2", "Heading 3"):
            size, bold, spacing = SECTION_PT, True, 1.5
            align = WD_ALIGN_PARAGRAPH.LEFT
            fmt.space_before = Pt(12)
            fmt.space_after = Pt(6)
        elif style_name in ("Heading 4", "Heading 5", "Heading 6"):
            size, bold, spacing = SUBSECTION_PT, True, 1.5
            align = WD_ALIGN_PARAGRAPH.LEFT
            fmt.space_before = Pt(8)
            fmt.space_after = Pt(4)
        elif style_name == "Source Code":
            size, bold, spacing = CODE_PT, False, 1.0
            align = WD_ALIGN_PARAGRAPH.LEFT
            fmt.space_before = Pt(4)
            fmt.space_after = Pt(4)
        elif style_name == "Image Caption":
            size, bold, spacing = CAPTION_PT, False, 1.0
            align = WD_ALIGN_PARAGRAPH.CENTER
            fmt.space_before = Pt(2)
            fmt.space_after = Pt(8)
        elif style_name == "Captioned Figure":
            size, bold, spacing = None, False, 1.0
            align = WD_ALIGN_PARAGRAPH.CENTER
            fmt.space_before = Pt(6)
            fmt.space_after = Pt(2)
        else:
            size, bold, spacing = BODY_PT, False, 1.5
            fmt.space_before = Pt(0)
            fmt.space_after = Pt(0)

        fmt.line_spacing = spacing
        paragraph.alignment = align

        mono = style_name == "Source Code"
        style_paragraph_runs(paragraph, size, bold=bold, mono=mono)

        # 行内代码（Verbatim Char）保持等宽字体；代码块本身不覆盖字号
        if style_name != "Source Code":
            for run in paragraph.runs:
                if run.style is not None and run.style.name == "Verbatim Char":
                    set_run_fonts(run, east=EAST, west=MONO)
                    run.font.size = Pt(CODE_PT + 1)

    # 图片缩放到版心宽度以内并居中
    max_width = Cm(15.2)
    for shape in doc.inline_shapes:
        if shape.width > max_width:
            ratio = max_width / shape.width
            shape.width = max_width
            shape.height = int(shape.height * ratio)

    # 表格：网格线、五号、表头加粗并加浅灰底
    for table in doc.tables:
        try:
            table.style = "Table Grid"
        except KeyError:
            pass
        table.alignment = WD_TABLE_ALIGNMENT.CENTER
        table.autofit = True
        set_table_borders(table)
        for row_index, row in enumerate(table.rows):
            if row_index == 0:
                for cell in row.cells:
                    shade_cell(cell)
            for cell in row.cells:
                for paragraph in cell.paragraphs:
                    paragraph.paragraph_format.line_spacing = 1.0
                    paragraph.paragraph_format.space_before = Pt(1)
                    paragraph.paragraph_format.space_after = Pt(1)
                    paragraph.alignment = (
                        WD_ALIGN_PARAGRAPH.CENTER
                        if row_index == 0
                        else WD_ALIGN_PARAGRAPH.LEFT
                    )
                    style_paragraph_runs(
                        paragraph, TABLE_PT, bold=(row_index == 0), mono=False
                    )

    add_page_number(section)

    doc.core_properties.title = "图 Baseline 实验报告"
    doc.core_properties.author = "甘锦珊"
    doc.save(DST)
    print("saved:", DST)


if __name__ == "__main__":
    main()
