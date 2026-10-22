import importlib.util
import tempfile
import unittest
from pathlib import Path

from pypdf import PdfReader


HERE = Path(__file__).resolve().parent
BUILDER_PATH = HERE / "build_riscv_llvm_backend_tutorial.py"


def load_builder():
    spec = importlib.util.spec_from_file_location("tutorial_pdf_builder", BUILDER_PATH)
    if spec is None or spec.loader is None:
        raise RuntimeError("cannot load PDF builder")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class TutorialPdfBuilderTest(unittest.TestCase):
    def test_build_pdf_renders_supported_markdown_as_searchable_a4(self):
        """Catches dropped Chinese text, unsupported blocks, or wrong page size."""
        markdown = """# 中文教程标题

> 这是阅读提示。

## 第一章

正文包含 `MachineInstr` 和中文。

- 列表项目一
- 列表项目二

```text
LLVM IR -> SelectionDAG -> MachineInstr -> MCInst
中文注释 -> vsetvli
```

| 名称 | 作用 |
|---|---|
| AVL | 请求的向量长度 |

### 小结

结束。
"""
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            source = temp / "input.md"
            output = temp / "output.pdf"
            source.write_text(markdown, encoding="utf-8")

            load_builder().build_pdf(str(source), str(output))

            self.assertTrue(output.read_bytes().startswith(b"%PDF-"))
            reader = PdfReader(str(output))
            self.assertGreaterEqual(len(reader.pages), 1)
            page = reader.pages[0]
            self.assertAlmostEqual(float(page.mediabox.width), 595.28, delta=1.0)
            self.assertAlmostEqual(float(page.mediabox.height), 841.89, delta=1.0)
            text = "\n".join(p.extract_text() or "" for p in reader.pages)
            for expected in (
                "中文教程标题",
                "这是阅读提示",
                "MachineInstr",
                "LLVM IR -> SelectionDAG -> MachineInstr -> MCInst",
                "中文注释 -> vsetvli",
                "请求的向量长度",
                "小结",
            ):
                self.assertIn(expected, text)
            self.assertNotIn("1\n列表项目一", text)
            self.assertIn("-\n列表项目一", text)

    def test_next_core_chapter_uses_available_page_space(self):
        """Catches an almost-empty page caused by an unconditional chapter break."""
        body = "\n\n".join(f"短段落 {index}" for index in range(30))
        markdown = f"""# 分页测试

## 先建立后端地图

{body}

尾部标记

## 案例一：整数加法如何变成 add

下一章正文。
"""
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            source = temp / "input.md"
            output = temp / "output.pdf"
            source.write_text(markdown, encoding="utf-8")

            load_builder().build_pdf(str(source), str(output))

            pages = [page.extract_text() or "" for page in PdfReader(str(output)).pages]
            tail_page = next(page for page in pages if "尾部标记" in page)
            self.assertIn("案例一：整数加法如何变成 add", tail_page)

    def test_full_tutorial_does_not_leave_a_sparse_trailing_page(self):
        """Catches a reference list spilling only two items onto a new page."""
        source = HERE.parents[1] / "rvv" / "RISC-V_LLVM后端源码阅读教程.md"
        with tempfile.TemporaryDirectory() as temp_dir:
            output = Path(temp_dir) / "tutorial.pdf"

            load_builder().build_pdf(str(source), str(output))

            last_page = PdfReader(str(output)).pages[-1].extract_text() or ""
            content_lines = [
                line for line in last_page.splitlines()
                if line.strip()
                and "RISC-V LLVM 后端源码阅读教程" not in line
                and not line.startswith("第 ")
            ]
            self.assertGreaterEqual(len(content_lines), 8)


if __name__ == "__main__":
    unittest.main()
