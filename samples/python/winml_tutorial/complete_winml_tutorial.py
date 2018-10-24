import sys
vi = sys.version_info
dirname = "lib.{2}-{0}.{1}".format(vi.major, vi.minor, "win-amd64" if sys.maxsize > 2**32 else "win32")
test_module_path = "./pywinrt_output/build/" + dirname
sys.path.append(str(test_module_path))

import _pyrt
import asyncio

def timed_op(fun):
    import time

    def sync_wrapper(*args, **kwds):
        print("Starting", fun.__name__)
        start = time.perf_counter()

        ret = fun(*args, **kwds)

        end = time.perf_counter()
        print(fun.__name__, "took", end - start, "seconds")
        return ret

    async def async_wrapper(*args, **kwds):
        print("Starting", fun.__name__)
        start = time.perf_counter()

        ret = await fun(*args, **kwds)

        end = time.perf_counter()
        print(fun.__name__, "took", end - start, "seconds")
        return ret

    return async_wrapper if asyncio.iscoroutinefunction(fun) else sync_wrapper

async def wrap_async_op(op):
    loop = asyncio.get_event_loop()
    future = loop.create_future()

    def callback(operation, status):
        if status == 1:
            result = operation.GetResults()
            loop.call_soon_threadsafe(asyncio.Future.set_result, future, result)
        elif status == 2:
            loop.call_soon_threadsafe(asyncio.Future.set_exception, future, asyncio.CancelledError())
        elif status == 3:
            loop.call_soon_threadsafe(asyncio.Future.set_exception, future, RuntimeError("AsyncOp failed"))
        else:
            loop.call_soon_threadsafe(asyncio.Future.set_exception, future, RuntimeError("Unexpected AsyncStatus"))

    op.put_Completed(callback)

    return await future

def run_async_code(code):
    loop = asyncio.get_event_loop()
    loop.run_until_complete(code())
    loop.close()

@timed_op
def load_model(model_path):
    return _pyrt.LearningModel.LoadFromFilePath(model_path)

@timed_op
async def load_image_file(file_path):
    file = await wrap_async_op(_pyrt.StorageFile.GetFileFromPathAsync(file_path))
    stream = await wrap_async_op(file.OpenAsync(0)) # 0 == FileAccessMode::Read 
    decoder = await wrap_async_op(_pyrt.BitmapDecoder.CreateAsync(stream))
    software_bitmap = await wrap_async_op(decoder.GetSoftwareBitmapAsync())
    return _pyrt.VideoFrame.CreateWithSoftwareBitmap(software_bitmap)

@timed_op
def bind_model(model, image_frame):
    device = _pyrt.LearningModelDevice(0) # 0 == LearningModelDeviceKind::Default
    session = _pyrt.LearningModelSession(model, device)
    binding = _pyrt.LearningModelBinding(session)
    image_feature_value = _pyrt.ImageFeatureValue.CreateFromVideoFrame(image_frame)
    binding.Bind("data_0", image_feature_value)
    shape = _pyrt.TensorFloat.Create([1, 1000, 1, 1])
    binding.Bind("softmaxout_1", shape)
    return (session, binding)

@timed_op
def evaluate_model(session, binding):
    results = session.Evaluate(binding, "RunId")
    o = results.get_Outputs().Lookup("softmaxout_1")
    result_tensor = _pyrt.TensorFloat._from(o)
    return result_tensor.GetAsVectorView()

def load_labels(labels_path):
    import csv
    labels = dict()
    with open(labels_path) as labels_file:
        labels_reader = csv.reader(labels_file)
        for label in labels_reader:
            label_text = ', '.join(label[1:])
            labels[int(label[0])] = ', '.join(label[1:])
    return labels

def print_results(results, labels):
    topProbabilities = [0.0 for x in range(3)]
    topProbabilityLabelIndexes = [0 for x in range(3)]

    for i in range(results.get_Size()):
        for j in range(3):
            result = results.GetAt(i)
            if result > topProbabilities[j]:
                topProbabilityLabelIndexes[j] = i
                topProbabilities[j] = result
                break

    print()
    for i in range(3):
        print(labels[topProbabilityLabelIndexes[i]], "with confidence of", topProbabilities[i])

async def async_main():
    import os.path
    
    model_path = os.path.abspath("./winml_content/SqueezeNet.onnx")
    model = load_model(model_path)

    image_file = os.path.abspath("./winml_content/kitten_224.png")
    image_frame = await load_image_file(image_file)

    session, binding = bind_model(model, image_frame)
    results = evaluate_model(session, binding)

    labels_path = os.path.abspath("./winml_content/Labels.txt")
    labels = load_labels(labels_path)

    print_results(results, labels)



_pyrt.init_apartment()

run_async_code(async_main)