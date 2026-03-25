import torch
import torch_mlir
from transformers import AutoModelForCausalLM, AutoTokenizer

model_dir = "/path/to/your/model"  # 含 config.json + model.safetensors/.bin
模型结构超参数
层数（num_hidden_layers）
隐藏维度（hidden_size）
注意力头数（num_attention_heads）
FFN 中间维度（intermediate_size）
最大位置长度（max_position_embeddings）
架构类型与类名
model_type
architectures（如 Qwen2ForCausalLM / LlamaForCausalLM）
数值与算子配置
torch_dtype（float16/bfloat16 等）
rms_norm_eps / layer_norm_eps
rope_theta、位置编码参数
tie_word_embeddings 等
词表相关（部分在 tokenizer 配置）
vocab_size
bos/eos/pad_token_id
任务与生成默认参数（可选）
是否是 causal LM、encoder-decoder
一些 generation 默认值（有时在 generation_config.json）
简化说：
config 决定“网络长什么样”，权重文件决定“参数具体是多少”。
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
