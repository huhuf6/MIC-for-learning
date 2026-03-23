import torch
import torch_mlir
from transformers import AutoModelForCausalLM, AutoTokenizer

model_dir = "/path/to/your/model"  # 含 config.json + model.safetensors/.bin

# 1) 加载 tokenizer + 模型权重
tokenizer = AutoTokenizer.from_pretrained(model_dir, trust_remote_code=True)
model = AutoModelForCausalLM.from_pretrained(
    model_dir,
    torch_dtype=torch.float16,   # 可按需改成 float32
    device_map="cpu",
    trust_remote_code=True,
).eval()

# 2) 准备示例输入（决定导出签名）
inputs = tokenizer("你好，介绍一下你自己。", return_tensors="pt")
input_ids = inputs["input_ids"]
attention_mask = inputs.get("attention_mask")

# 注意：不同模型 forward 参数不同，这里给常见 CausalLM 两参示例
example_args = (input_ids, attention_mask) if attention_mask is not None else (input_ids,)

# 3) 导出 torch-mlir
# 输出 torch 层 IR（torch/torch_c）
mlir_torch = torch_mlir.compile(
    model,
    example_args,
    output_type="torch",
)
with open("model_torch.mlir", "w", encoding="utf-8") as f:
    f.write(str(mlir_torch))

# 输出 linalg-on-tensors IR（更接近后端）
mlir_linalg = torch_mlir.compile(
    model,
    example_args,
    output_type="linalg-on-tensors",
)
with open("model_linalg.mlir", "w", encoding="utf-8") as f:
    f.write(str(mlir_linalg))

print("done: model_torch.mlir, model_linalg.mlir")
