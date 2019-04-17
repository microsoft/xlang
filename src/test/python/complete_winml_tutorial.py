import sys
sys.path.append("./generated")

import winrt
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

import winrt.windows.ai.machinelearning as winml 
import os

@timed_op
def load_model(model_path):
    return winml.LearningModel.load_from_file_path(os.fspath(model_path))

@timed_op
async def load_image_file(file_path):
    from winrt.windows.storage import StorageFile, FileAccessMode
    from winrt.windows.graphics.imaging import BitmapDecoder
    from winrt.windows.media import VideoFrame

    file = await StorageFile.get_file_from_path_async(os.fspath(file_path))
    stream = await file.open_async(FileAccessMode.READ) 
    decoder = await BitmapDecoder.create_async(stream)
    software_bitmap = await decoder.get_software_bitmap_async()
    return VideoFrame.create_with_software_bitmap(software_bitmap)

@timed_op
def bind_model(model, image_frame):
    device = winml.LearningModelDevice(winml.LearningModelDeviceKind.DEFAULT)
    session = winml.LearningModelSession(model, device)
    binding = winml.LearningModelBinding(session)
    image_feature_value = winml.ImageFeatureValue.create_from_video_frame(image_frame)
    binding.bind("data_0", image_feature_value)
    shape = winml.TensorFloat.create([1, 1000, 1, 1])
    binding.bind("softmaxout_1", shape)
    return (session, binding)

@timed_op
def evaluate_model(session, binding):
    results = session.evaluate(binding, "RunId")
    o = results.outputs.lookup("softmaxout_1")
    result_tensor = winml.TensorFloat._from(o)
    return result_tensor.get_as_vector_view()

def load_labels(labels_path):
    import csv
    labels = dict()
    with open(os.fspath(labels_path)) as labels_file:
        labels_reader = csv.reader(labels_file)
        for label in labels_reader:
            label_text = ', '.join(label[1:])
            labels[int(label[0])] = ', '.join(label[1:])
    return labels

def print_results(results, labels):
    topProbabilities = [0.0 for x in range(3)]
    topProbabilityLabelIndexes = [0 for x in range(3)]

    for i, result in enumerate(results):
        for j in range(3):
            if result > topProbabilities[j]:
                topProbabilityLabelIndexes[j] = i
                topProbabilities[j] = result
                break

    print()
    for i in range(3):
        print(labels[topProbabilityLabelIndexes[i]], "with confidence of", topProbabilities[i])

async def async_main():

    from pathlib import Path
    cur_path = Path.cwd()

    while (cur_path / "samples").is_dir() == False:
        cur_path = cur_path.parent

    winml_content_path = cur_path / "samples/python/winml_tutorial/winml_content"

    model_path = winml_content_path / "SqueezeNet.onnx"
    model = load_model(model_path)

    image_file =  winml_content_path / "kitten_224.png"
    image_frame = await load_image_file(image_file)

    session, binding = bind_model(model, image_frame)
    results = evaluate_model(session, binding)

    labels_path = winml_content_path / "Labels.txt"
    labels = load_labels(labels_path)

    print_results(results, labels)

loop = asyncio.get_event_loop()
loop.run_until_complete(async_main())
loop.close()