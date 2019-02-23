// Java WinRT projection general support

#pragma once

#include "pch.h"
#include "jni.h"
#include <variant>

#include "winrt/Windows.Foundation.h"

#define JNI_EXPORT_IMPL(jni_export, jni_class) \
    extern "C" \
    JNIEXPORT void JNICALL \
    jni_export(jni_env* env, jclass cls) noexcept try \
    { \
        jni_class::Register(*env, cls); \
    } \
    catch (...) \
    { \
        env->raise_java_exception(#jni_export); \
    }

#define JNI_EXPORT_FULLNAME_IMPL(jni_namespace, jni_class) \
    jni_namespace##_##jni_class##_##Register 

#define JNI_EXPORT_FULLNAME(jni_namespace, jni_class) \
	JNI_EXPORT_FULLNAME_IMPL(jni_namespace, jni_class)

#define JNI_EXPORT(jni_class) \
    JNI_EXPORT_IMPL(JNI_EXPORT_FULLNAME(JNI_EXPORT_NAMESPACE, jni_class), jni_class) 

#define JNI_CHECK(Expr) do { auto jerr = Expr; if(jerr < JNI_OK){ return jerr; } } while(0)

#define JNI_IMPL(Func, Sig) { const_cast<char*>(#Func), const_cast<char*>(Sig), Func }

static JavaVM* cached_jvm = nullptr;

// This structure provides an efficient agile reference to the wrapped abi.
// If the underlying object is agile, no QIs and help allocations are necessary.
// Otherwise, if not agile, a shared agile_abi_ref is created on the heap 
// to contain an agile reference, ref count, and the necessary resolving guid.
struct agile_abi_ref
{
	// for a direct agile abi, the first 'member' is the vfptr
	enum class abi_type
	{
		non_agile = 1,    // value distinct from any vfptr 
	}
	_type{ abi_type::non_agile };
	winrt::guid _guid;
	winrt::impl::IAgileReference* _ref;
	std::atomic<uint32_t> _refs{ 1 };

	template<typename T>
	agile_abi_ref(T value)
	{
		_guid = winrt::guid_of<T>();
		winrt::check_hresult(WINRT_RoGetAgileReference(0, _guid, winrt::get_abi(value), (void**)&_ref));
	}

	template<typename T>
	static agile_abi_ref* create(T value)
	{
		if (value.try_as<::IAgileObject>())
		{
			// _type (i.e., abi's vfptr) will never equal abi_type::non_agile
			return (agile_abi_ref*)winrt::detach_abi(value);
		}
		return new agile_abi_ref(value);
	}

	static auto from(jlong abi)
	{
		return reinterpret_cast<agile_abi_ref*>(abi);
	}

	void addref() noexcept
	{
		if (_type == abi_type::non_agile)
		{
			_refs.fetch_add(1, std::memory_order_relaxed);
			return;
		}
		as_agile()->AddRef();
	}

	void release() noexcept
	{
		if (_type == abi_type::non_agile)
		{
			uint32_t const remaining = _refs.fetch_sub(1, std::memory_order_relaxed) - 1;
			if (remaining == 0)
			{
				delete this;
			}
			return;
		}
		as_agile()->Release();
	}

	void* resolve()
	{
		if (_type == abi_type::non_agile)
		{
			::IUnknown* abi;
			winrt::check_hresult(_ref->Resolve(_guid, (void**)&abi));
			return abi;
		}
		as_agile()->AddRef();
		return this;
	}

private:
	::IUnknown* as_agile() noexcept
	{
		return reinterpret_cast<::IUnknown*>(this);
	}
};

template<typename T>
jlong create_agile_ref(T value)
{
	return reinterpret_cast<jlong>(agile_abi_ref::create(value));
}

// Helper for converting from a jstring to a std::wstring_view for efficient WinRT pass thru
struct jstring_view : public std::wstring_view
{
	JNIEnv& _env;
	jstring _jstr;

	jstring_view(JNIEnv& env, jstring jstr)
		: std::wstring_view{ get_chars(env, jstr), env.GetStringLength(jstr) }, _env(env), _jstr(jstr)
	{
	}

	~jstring_view()
	{
		_env.ReleaseStringCritical(_jstr, (jchar const*)data());
	}

private:
	static wchar_t const * get_chars(JNIEnv& env, jstring jstr)
	{
		if (jboolean isCopy; auto chars = env.GetStringCritical(jstr, &isCopy))
		{
			return (wchar_t const *)chars;
		}
		winrt::throw_hresult(winrt::impl::error_bad_alloc);
	}
};

struct jni_env : JNIEnv
{
	template<size_t count>
	auto register_natives(jclass cls, const JNINativeMethod(&methods)[count])
	{
#ifdef _DEBUG
		jint result = JNI_OK;
		for (auto& method : methods)
		{
			result = RegisterNatives(cls, &method, 1);
			if (result != JNI_OK)
			{
				if (IsDebuggerPresent())
				{
					auto error = std::string("Error registering method: ") + method.name + " with signature: " + method.signature + "\n";
					OutputDebugStringA(error.c_str());
					DebugBreak();
				}
				break;
			}
		}
#else
		auto result = RegisterNatives(cls, methods, _countof(methods));
#endif
		if (result != JNI_OK)
		{
			throw winrt::hresult_class_not_available();
		}
		return result;
	}

	jstring to_jstring(winrt::hstring const& hstr) noexcept
	{
		return NewString((jchar const*)hstr.data(), hstr.size());
	}

	jstring to_jstring(std::wstring_view str) noexcept
	{
		return NewString((jchar const*)str.data(), (jsize)str.size());
	}

	jstring to_jstring(char const *str) noexcept
	{
		return NewStringUTF(str);
	}

	template<size_t size>
	jstring to_jstring(wchar_t const(&str)[size]) noexcept
	{
		return NewString((jchar const*)str, size - 1);
	}

	auto find_class(char const* name)
	{
		if (auto cls = FindClass(name))
		{
			return cls;
		}
		throw winrt::hresult_class_not_available();
	}

	auto get_class(jobject obj)
	{
		if (auto cls = GetObjectClass(obj))
		{
			return cls;
		}
		throw winrt::hresult_class_not_available();
	}

	auto get_field_id(jclass cls, char const* field, char const* sig)
	{
		if (auto field_id = GetFieldID(cls, field, sig))
		{
			return field_id;
		}
		throw winrt::hresult_not_implemented();
	}

	auto get_method_id(jclass cls, char const* method, char const* sig)
	{
		if (auto method_id = GetMethodID(cls, method, sig))
		{
			return method_id;
		}
		throw winrt::hresult_not_implemented();
	}

	auto get_method_id(jobject obj, char const* method, char const* sig)
	{
		return get_method_id(get_class(obj), method, sig);
	}

	auto cache_class(char const* className) noexcept
	{
		auto cls = FindClass(className);
		return cls ? NewWeakGlobalRef(cls) : nullptr;
	}

	template<typename T = jobject>
	auto try_resolve_weakref(jweak weak) noexcept
	{
		return static_cast<T>(IsSameObject(weak, nullptr) ? nullptr : weak);
	}

	template<typename T = jobject>
	auto resolve_weakref(jweak weak)
	{
		if (auto value = try_resolve_weakref(weak))
		{
			return (T)(value);
		}
		throw winrt::hresult_changed_state();
	}

	template<typename T>
	T get_enum_value(jobject jenum)
	{
		return static_cast<T>(CallIntMethod(jenum, get_method_id(jenum, "ordinal", "()I")));
	}

	static jni_env* try_get(JavaVM* jvm = cached_jvm) noexcept
	{
		jni_env* env;
		return jvm->GetEnv((void **)&env, JNI_VERSION_1_2) ? nullptr : env;
	}

	static jni_env& get(JavaVM* jvm = cached_jvm)
	{
		if (auto env = try_get(jvm))
		{
			return *env;
		}
		throw winrt::hresult_wrong_thread();
	}

	using class_variant = std::variant<jclass, jstring, char const*>;

	auto resolve_class(class_variant const& cls_var)
	{
		if (auto pcls = std::get_if<jclass>(&cls_var))
		{
			return *pcls;
		}
		if (auto pstr = std::get_if<jstring>(&cls_var))
		{
			auto jstr = jstring_view(*this, *pstr);
			auto str = std::string(jstr.begin(), jstr.end());
			return find_class(str.data());
		}
		return find_class(std::get<char const*>(cls_var));
	}

	auto create_projected_var(class_variant const& cls_var, char const* init_sig, jlong abi, ...)
	{
		try
		{
			auto cls = resolve_class(cls_var);
			auto init_id = get_method_id(cls, "<init>", init_sig);
			va_list abi_and_params;
			va_start(abi_and_params, init_sig);
			auto jobj = NewObjectV(cls, init_id, abi_and_params);
			va_end(abi_and_params);
			if (jobj)
			{
				return jobj;
			}
		}
		catch (...)
		{
		}
		// on error, don't leak the detached object
		agile_abi_ref::from(abi)->release();
		return jobject{};
	}

    template<typename P, typename T>
	auto create_projected_object(T obj)
	{
		return create_projected_var(P::projected_type, "(J)V", create_agile_ref(obj));
	}

	template<typename T>
	auto create_projected_object(T obj, class_variant const& cls_var)
	{
		return create_projected_var(cls_var, "(J)V", create_agile_ref(obj));
	}

	template<typename T>
	auto create_projected_object(T obj, class_variant const& cls_var, jstring param0)
	{
		return create_projected_var(cls_var, "(JLjava/lang/String;)V", create_agile_ref(obj), param0);
	}

	template<typename T>
	auto create_projected_object(T obj, class_variant const& cls_var, jlong param0)
	{
		return create_projected_var(cls_var, "(JJ)V", create_agile_ref(obj), param0);
	}

	template<typename T>
	auto create_projected_object(T obj, class_variant const& cls_var, jlong param0, jlong param1)
	{
		return create_projected_var(cls_var, "(JJJ)V", create_agile_ref(obj), param0, param1);
	}

	template<typename T>
	auto create_projected_object(T obj, class_variant const& cls_var, jlong param0, jlong param1, jlong param2)
	{
		return create_projected_var(cls_var, "(JJJJ)V", create_agile_ref(obj), param0, param1, param2);
	}

	template<typename T>
	auto create_projected_object(T obj, class_variant const& cls_var, jlong param0, jlong param1, jlong param2, jlong param3)
	{
		return create_projected_var(cls_var, "(JJJJJ)V", create_agile_ref(obj), param0, param1, param2, param3);
	}

	template<typename D, typename T>
	jobject create_projected_generic(T obj)
	{
		return create_projected_object(obj, to_jstring(D::element_type));
	}

	template<typename T>
	jobject create_projected_observable(T& obs_obj, char const* elementType);

	template<typename T>
	jobject create_projected_future(T& async_obj, char const* resultType);

	auto find_java_exception(char const * className) noexcept
	{
		if (auto cls_exception = FindClass(className))
		{
			return cls_exception;
		}
		return FindClass("java/lang/Exception");
	}

	auto create_java_exception(char const * className, jstring message) noexcept
	{
		if (auto cls_exception = find_java_exception(className))
		{
			auto init_id = get_method_id(cls_exception, "<init>", "(Ljava/lang/String;)V");
			return NewObject(cls_exception, init_id, message);
		}
		return jobject{};
	}

	auto raise_java_exception(char const * className, char const * message) noexcept
	{
		if (auto cls_exception = find_java_exception(className))
		{
			ThrowNew(cls_exception, message);
		}
	}

	auto raise_java_exception(char const * context) noexcept
	{
		char const* className;
		try
		{
			throw;
		}
		catch (winrt::hresult_access_denied&)
		{
			className = "java/lang/IllegalAccessException";
		}
		catch (winrt::hresult_wrong_thread&)
		{
			className = "java/lang/IllegalThreadStateException";
		}
		catch (winrt::hresult_not_implemented&)
		{
			className = "java/lang/UnsupportedOperationException";
		}
		catch (winrt::hresult_invalid_argument&)
		{
			className = "java/lang/IllegalArgumentException";
		}
		catch (winrt::hresult_out_of_bounds&)
		{
			className = "java/lang/IndexOutOfBoundsException";
		}
		catch (winrt::hresult_no_interface&)
		{
			className = "java/lang/ClassCastException";
		}
		catch (winrt::hresult_class_not_available&)
		{
			className = "java/lang/ClassNotFoundException";
		}
		catch (winrt::hresult_changed_state&)
		{
			className = "java/lang/IllegalStateException";
		}
		catch (winrt::hresult_illegal_method_call&)
		{
			className = "java/lang/IllegalStateException";
		}
		catch (winrt::hresult_illegal_state_change&)
		{
			className = "java/lang/IllegalStateException";
		}
		catch (winrt::hresult_canceled&)
		{
			className = "java/lang/IllegalStateException";
		}
		catch (winrt::hresult_error&)
		{
			className = "java/lang/RuntimeException";
		}
		catch (std::system_error&)
		{
			className = "java/lang/RuntimeException";
		}
		catch (std::bad_alloc&)
		{
			className = "java/lang/NullPointerException";
		}
		catch (std::length_error&)
		{
			className = "java/lang/IndexOutOfBoundsException";
		}
		catch (std::out_of_range&)
		{
			className = "java/lang/IndexOutOfBoundsException";
		}
		catch (std::range_error&)
		{
			className = "java/lang/ArithmeticException";
		}
		catch (std::overflow_error&)
		{
			className = "java/lang/ArithmeticException";
		}
		catch (std::underflow_error&)
		{
			className = "java/lang/ArithmeticException";
		}
		catch (std::invalid_argument&)
		{
			className = "java/lang/IllegalArgumentException";
		}
		catch (...)
		{
			className = "java/lang/Exception";
		}

		raise_java_exception(className, context);
	}
};

inline void raise_java_exception(char const * className, char const * context) noexcept
{
	if (auto env = jni_env::try_get())
	{
		env->raise_java_exception(className, context);
	}
}

inline void raise_java_exception(char const * context) noexcept
{
	if (auto env = jni_env::try_get())
	{
		env->raise_java_exception(context);
	}
}

#ifndef JAVA_COMPILER_GENERATED

template<typename Source, typename Target = Source>
struct jni_traits
{
	static Target to_jvalue(jni_env& /*env*/, Source value)
	{
		return (Target)value;
	}
};

template<>
struct jni_traits<void> {};

template<>
struct jni_traits<void*> : jni_traits<void*, jlong> {};

template<>
struct jni_traits<bool> : jni_traits<bool, jboolean> {};

template<>
struct jni_traits<int32_t> : jni_traits<int32_t, jint> {};

template<>
struct jni_traits<long> : jni_traits<long, jlong> {};

template<>
struct jni_traits<winrt::hstring>
{
	static auto to_jvalue(jni_env& env, winrt::hstring const& value)
	{
		return env.to_jstring(value);
	}
};

// Template needs to be able to capture compile-time constant impl func pointer,
// and also to deduce argument and return types for composition, thus nested templates.
template<typename impl_t, impl_t* impl>
struct jni_wrapper
{
	template<char const*(*get_name)(), typename result_t, typename context_t, typename... args_t>
	static auto make(result_t(*)(jni_env&, context_t, args_t...))
	{
		struct wrapper
		{
			// Easier to specify convention and try-catch with a function than a lambda
			static auto JNICALL invoke(jni_env* env, context_t obj, args_t... args) noexcept try
			{
				if constexpr (std::is_same_v<result_t, void>)
				{
					impl(*env, obj, args...);
				}
				else
				{
					return jni_traits<result_t>::to_jvalue(*env, std::move(impl(*env, obj, args...)));
				}
			}
			catch (...)
			{
				env->raise_java_exception(get_name());
				if constexpr (!std::is_same_v<result_t, void>)
				{
					return decltype(jni_traits<result_t>::to_jvalue(*env, impl(*env, obj, args...))){};
				}
			}
		};
		return wrapper::invoke;
	}
};

// Provide explicit mapping of function to name, to support overloading
#define JNI_METHODN(Func, Name, Sig) { \
    const_cast<char*>(Name), \
    const_cast<char*>(Sig), \
    jni_wrapper<decltype(Func),Func>::make<[]{return Name;}>(Func) }

// Default to function-name equivalence, no suffix
#define JNI_METHOD0(Func, Sig) \
    JNI_METHODN(Func, #Func, Sig)

// Overload 1, requiring function suffix "1"
#define JNI_METHOD1(Func, Sig) \
    JNI_METHODN(Func##1, #Func, Sig)

// Overload 2, requiring function suffix "2"
#define JNI_METHOD2(Func, Sig) \
    JNI_METHODN(Func##2, #Func, Sig)

// Overload 3, requiring function suffix "3"
#define JNI_METHOD3(Func, Sig) \
    JNI_METHODN(Func##3, #Func, Sig)

#endif

// todo: attach/detach current thread are expensive
struct jvm_thread
{
	jvm_thread()
	{
		auto jerr = cached_jvm->AttachCurrentThread((void**)&env, nullptr);
		if (jerr < JNI_OK)
		{
			raise_java_exception("java/lang/RuntimeException", "callback");
		}
	}

	~jvm_thread()
	{
		cached_jvm->DetachCurrentThread();
	}

	jni_env* env;
};

template <typename T>
struct future_traits
{
	static constexpr bool has_progress = false;
};

template <typename TResult, typename TProgress>
struct future_traits<winrt::Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>>
{
	static constexpr bool has_progress = true;
};

#if 0
template<typename T>
jobject jni_env::create_projected_observable(T& obs_obj, char const* elementType)
{
	// IObservableVector<T> models observability, and iteration (IIterable<T>)  
	// but not immutability (also inherits IVector<T>).
	// IVector projects as IList in C# and has list operations (not fixed array)
	// IVectorView models immutability, and can be obtained via IVector::GetView()
	// Some implementations may choose not to implement (throw) the mutable part of IVector
	// We should just pass this along, but might need to adapt to Java exceptions
	// Any Java collection can be made immutable, by wrapping via Collections.unmodifiable*() 
	// ObservableList<E> interface inherits Collection<E>, Iterable<E>, List<E>, Observable.
	// Java provides ObservableArray<T extends ObservableArray<T>> interface to adorn an existing
	// array collection with observability (add/remove listener) corresponding to IObservableVector events.
	//
	// create java "Windows/Foundation/ObservableList" to own the underlying observable vector
	// and implement ObservableList<E>.

	auto jobs = create_projected_object(obs_obj, "Windows/Foundation/ObservableList");
	jweak jobs_weakref = NewWeakGlobalRef(jobs);

	obs_obj.VectorChanged([jobs_weakref, elementType](auto& sender, auto& status) {
		//jvm_thread thread;
		//auto& env = *thread.env;
		//if (auto jfuture = env.resolve_weakref(jfuture_weakref))
		//{
		//    auto cls = env.find_class("java/util/concurrent/CompletableFuture");
		//    auto complete_id = env.get_method_id(cls, "complete", "(Ljava/lang/Object;)Z");
		//    auto result_obj = sender.GetResults();
		//    // todo: support for non-class results
		//    auto result = env.create_projected_object(resultType, result_obj);
		//    env.CallBooleanMethod(jfuture, complete_id, result);
		//}
		});

	return jobs;
}
#endif

template<typename T>
jobject jni_env::create_projected_future(T& async_obj, char const* resultType)
{
	auto async = create_projected_object(async_obj, "Windows/Foundation/AsyncOperationWithProgress");

	// return value is a future adapter that holds a reference to an async operation
	// The reference type, AsyncOperationWithProgress, accomplishes two things:
	// 1. Keeps the async operation alive as long as the CompletableFuture is 
	// 2. Bridges the async operation's callbacks to the CompletableFuture
	// Due to the circularity, AsyncOperationWithProgress handlers must hold a weak ref to 
	// the future, to allow garbage collection.
	auto cls_future = find_class("Windows/Foundation/Future");
	// Look up Future(AsyncOperationWithProgress) constructor
	auto init_id = get_method_id(cls_future, "<init>", "(LWindows/Foundation/AsyncOperationWithProgress;)V");
	auto jfuture = NewObject(cls_future, init_id, async);
	jweak jfuture_weakref = NewWeakGlobalRef(jfuture);

	async_obj.Completed([jfuture_weakref, resultType](auto& sender, winrt::Windows::Foundation::AsyncStatus status)
		{
			switch (status)
			{
			case winrt::Windows::Foundation::AsyncStatus::Started:
				break;
			case winrt::Windows::Foundation::AsyncStatus::Completed:
			{
				jvm_thread thread;
				auto& env = *thread.env;
				if (auto jfuture = env.resolve_weakref(jfuture_weakref))
				{
					auto cls = env.find_class("java/util/concurrent/CompletableFuture");
					auto complete_id = env.get_method_id(cls, "complete", "(Ljava/lang/Object;)Z");
					auto result_obj = sender.GetResults();
					// todo: support for non-class results
					auto result = env.create_projected_object(result_obj, resultType);
					env.CallBooleanMethod(jfuture, complete_id, result);
				}
			}
			break;
			case winrt::Windows::Foundation::AsyncStatus::Canceled:
				break;
			case winrt::Windows::Foundation::AsyncStatus::Error:
			{
				jvm_thread thread;
				auto& env = *thread.env;
				if (auto jfuture = env.resolve_weakref(jfuture_weakref))
				{
					auto cls = env.find_class("java/util/concurrent/CompletableFuture");
					auto complete_id = env.get_method_id(cls, "completeExceptionally", "(Ljava/lang/Throwable;)Z");
					// unfortunately, some layers in the stack (e.g,. wininet) eat the originating error 
					auto error = winrt::hresult_error(sender.ErrorCode(), winrt::hresult_error::from_abi);
					auto message = env.to_jstring(error.message());
					auto exception = env.create_java_exception("java/lang/Exception", message);
					env.CallBooleanMethod(jfuture, complete_id, exception);
				}
			}
			break;
			}
		});

	if constexpr (future_traits<T>::has_progress)
	{
		// todo: Progress, IAsyncAction, etc
		async_obj.Progress([jfuture_weakref](auto& /*sender*/, auto& /*progress*/)
			{
				jvm_thread thread;
				auto& env = *thread.env;
				//progress.BytesRetrieved;
				//progress.TotalBytesToRetrieve;
				auto jfuture = env.resolve_weakref(jfuture_weakref);
				if (jfuture)
				{
					// update...
				}
			});
	}

	return jfuture;
}

template<typename T>
struct resolve : public T
{
	resolve(jlong abi) : T{ nullptr }
	{
		winrt::attach_abi(*this, agile_abi_ref::from(abi)->resolve());
	}
};


template<typename T>
struct Projection
{
	static inline jweak class_cached = nullptr;
	using type = T;
	using resolve = ::resolve<type>;

	static void Register(jni_env& env, jclass cls)
	{
		class_cached = env.NewWeakGlobalRef(cls);
	}

	static void Unregister(jni_env& env) noexcept
	{
		env.DeleteWeakGlobalRef(class_cached);
	}
};

template<typename D>
struct Iterator
{
	static bool abi_hasNext(jni_env&, jobject, jlong abi)
	{
		return D::resolve{ abi }.HasCurrent();
	}

	static jobject abi_next(jni_env& env, jobject, jlong abi)
	{
		auto obj = D::resolve{ abi };
		if (!obj.HasCurrent())
		{
			throw winrt::hresult_illegal_method_call();
		}
		auto value_obj = obj.Current();
		obj.MoveNext();
		return env.create_projected_generic<D>(value_obj);
	}

	static void Register(jni_env& env, jclass cls)
	{
		static JNINativeMethod methods[] =
		{
			JNI_METHOD0(abi_hasNext, "(J)Z"),
			JNI_METHOD0(abi_next, winrt::impl::combine("(J)L", D::element_type, ";\0").data()),
		};
		env.register_natives(cls, methods);
	}
};

template<typename D>
struct Iterable
{
	static jobject abi_iterator(jni_env& env, jobject, jlong abi)
	{
		return env.create_projected_object(D::resolve{ abi }.First(), D::iterator_type);
	}

	static void Register(jni_env& env, jclass cls)
	{
		static JNINativeMethod methods[] =
		{
			JNI_METHOD0(abi_iterator, "(J)Ljava/util/Iterator;"),
		};
		env.register_natives(cls, methods);
	}
};

template<typename D>
struct Collection : public Iterable<D>
{
	static jboolean abi_add(jni_env&, jobject, jlong abi, jlong arg0_abi)
	{
		abi;
		arg0_abi;
		return JNI_TRUE;
	}

	static jboolean abi_addAll(jni_env&, jobject, jlong abi, jobject arg0)
	{
		abi;
		arg0;
		return JNI_TRUE;
	}

	static void abi_clear(jni_env&, jobject, jlong abi)
	{
		abi;
	}

	static jboolean abi_contains(jni_env&, jobject, jlong abi, jobject arg0)
	{
		abi;
		arg0;
		return JNI_TRUE;
	}

	static jboolean abi_containsAll(jni_env&, jobject, jlong abi, jobject arg0)
	{
		abi;
		arg0;
		return JNI_TRUE;
	}

	static jboolean abi_isEmpty(jni_env&, jobject, jlong abi)
	{
		abi;
		return JNI_TRUE;
	}

	static jboolean abi_remove(jni_env&, jobject, jlong abi, jobject arg0)
	{
		abi;
		arg0;
		return JNI_TRUE;
	}

	static jboolean abi_removeAll(jni_env&, jobject, jlong abi, jobject arg0)
	{
		abi;
		arg0;
		return JNI_TRUE;
	}

	static jboolean abi_retainAll(jni_env&, jobject, jlong abi, jobject arg0)
	{
		abi;
		arg0;
		return JNI_TRUE;
	}

	static jint abi_size(jni_env&, jobject, jlong abi)
	{
		abi;
		return 0;
	}

	static jobjectArray abi_toArray(jni_env&, jobject, jlong abi)
	{
		abi;
		return nullptr;
	}

	static jobjectArray abi_toArray1(jni_env&, jobject, jlong abi, jobjectArray arg0)
	{
		abi;
		arg0;
		return nullptr;
	}

	static void Register(jni_env& env, jclass cls)
	{
		__super::Register(env, cls);
		static JNINativeMethod methods[] =
		{
			JNI_METHOD0(abi_add, "(JJ)Z"),
			JNI_METHOD0(abi_addAll, "(JLjava/util/Collection;)Z"),
			JNI_METHOD0(abi_clear, "(J)V"),
			JNI_METHOD0(abi_contains, "(JLjava/lang/Object;)Z"),
			JNI_METHOD0(abi_containsAll, "(JLjava/util/Collection;)Z"),
			JNI_METHOD0(abi_isEmpty, "(J)Z"),
			JNI_METHOD0(abi_remove, "(JLjava/lang/Object;)Z"),
			JNI_METHOD0(abi_removeAll, "(JLjava/util/Collection;)Z"),
			JNI_METHOD0(abi_retainAll, "(JLjava/util/Collection;)Z"),
			JNI_METHOD0(abi_size, "(J)I"),
			JNI_METHOD0(abi_toArray, "(J)[Ljava/lang/Object;"),
			JNI_METHOD1(abi_toArray, "(J[Ljava/lang/Object;)[Ljava/lang/Object;"),
		};
		env.register_natives(cls, methods);
	}
};

template<typename D>
struct List : public Collection<D>
{
	static void abi_add(jni_env&, jobject, jlong abi, int arg0, jlong arg1_abi)
	{
		abi;
		arg0;
		arg1_abi;
	}

	static jboolean abi_addAll(jni_env&, jobject, jlong abi, int arg0, jobject arg1)
	{
		abi;
		arg0;
		arg1;
		return JNI_TRUE;
	}

	static jobject abi_get(jni_env& env, jobject, jlong abi, int arg0)
	{
		arg0;
		return env.create_projected_generic<D>(D::resolve{ abi }.GetAt(arg0));
	}

	static jint abi_indexOf(jni_env&, jobject, jlong abi, jobject arg0)
	{
		abi;
		arg0;
		return 0;
	}

	static jint abi_lastIndexOf(jni_env&, jobject, jlong abi, jobject arg0)
	{
		abi;
		arg0;
		return 0;
	}

	static jobject abi_listIterator(jni_env&, jobject, jlong abi)
	{
		abi;
		return nullptr;
	}

	static jobject abi_listIterator1(jni_env&, jobject, jlong abi, int arg0)
	{
		abi;
		arg0;
		return nullptr;
	}

	static jobject abi_remove(jni_env&, jobject, jlong abi, int arg0)
	{
		abi;
		arg0;
		return nullptr;
	}

	static jobject abi_set(jni_env&, jobject, jlong abi, int arg0, jlong arg1_abi)
	{
		abi;
		arg0;
		arg1_abi;
		return nullptr;
	}

	static jobject abi_subList(jni_env&, jobject, jlong abi, int arg0, int arg1)
	{
		abi;
		arg0;
		arg1;
		return nullptr;
	}

	static void Register(jni_env& env, jclass cls)
	{
		__super::Register(env, cls);
		static JNINativeMethod methods[] =
		{
			JNI_METHOD0(abi_add, "(JIJ)V"),
			JNI_METHOD0(abi_addAll, "(JILjava/util/Collection;)Z"),
			JNI_METHOD0(abi_get, winrt::impl::combine("(JI)L", D::element_type, ";\0").data()),
			JNI_METHOD0(abi_indexOf, "(JLjava/lang/Object;)I"),
			JNI_METHOD0(abi_lastIndexOf, "(JLjava/lang/Object;)I"),
			JNI_METHOD0(abi_listIterator, "(J)Ljava/util/ListIterator;"),
			JNI_METHOD1(abi_listIterator, "(JI)Ljava/util/ListIterator;"),
			JNI_METHOD0(abi_remove, winrt::impl::combine("(JI)L", D::element_type, ";\0").data()),
			JNI_METHOD0(abi_set, winrt::impl::combine("(JIJ)L", D::element_type, ";\0").data()),
			JNI_METHOD0(abi_subList, "(JII)Ljava/util/List;"),
		};
		env.register_natives(cls, methods);
	}
};

template<typename D>
struct ObservableList : List<D>
{
	static void abi_addListener(jni_env&, jobject, jlong abi, jobject arg0)
	{
		abi;
		arg0;
		//		D::resolve{ abi }.addListener(arg0);
	}

	static void abi_removeListener(jni_env&, jobject, jlong abi, jobject arg0)
	{
		abi;
		arg0;
	}

	static jboolean abi_addAll(jni_env&, jobject, jlong abi, jobjectArray arg0)
	{
		abi;
		arg0;
		return JNI_TRUE;
	}

	static void abi_addListener1(jni_env&, jobject, jlong abi, jobject arg0)
	{
		abi;
		arg0;
	}

	static void abi_remove(jni_env&, jobject, jlong abi, int arg0, int arg1)
	{
		abi;
		arg0;
		arg1;
	}

	static jboolean abi_removeAll(jni_env&, jobject, jlong abi, jobject arg0)
	{
		abi;
		arg0;
		return JNI_TRUE;
	}

	static void abi_removeListener1(jni_env&, jobject, jlong abi, jobject arg0)
	{
		abi;
		arg0;
	}

	static jboolean abi_retainAll(jni_env&, jobject, jlong abi, jobjectArray arg0)
	{
		abi;
		arg0;
		return JNI_TRUE;
	}

	static jboolean abi_setAll(jni_env&, jobject, jlong abi, jobjectArray arg0)
	{
		abi;
		arg0;
		return JNI_TRUE;
	}

	static jboolean abi_setAll1(jni_env&, jobject, jlong abi, jobject arg0)
	{
		abi;
		arg0;
		return JNI_TRUE;
	}

	static void Register(jni_env& env, jclass cls)
	{
		__super::Register(env, cls);
		static JNINativeMethod methods[] =
		{
			JNI_METHOD0(abi_addListener, "(JLjavafx/beans/InvalidationListener;)V"),
			JNI_METHOD0(abi_removeListener, "(vLjavafx/beans/InvalidationListener;)V"),
			JNI_METHOD0(abi_addAll, "(J[Ljava/lang/Object;)Z"),
			JNI_METHOD1(abi_addListener, "(JLjavafx/collections/ListChangeListener;)V"),
			JNI_METHOD0(abi_remove, "(JII)V"),
			JNI_METHOD0(abi_removeAll, "(J[Ljava/lang/Object;)Z"),
			JNI_METHOD1(abi_removeListener, "(JLjavafx/collections/ListChangeListener;)V"),
			JNI_METHOD0(abi_retainAll, "(J[Ljava/lang/Object;)Z"),
			JNI_METHOD0(abi_setAll, "(J[Ljava/lang/Object;)Z"),
			JNI_METHOD1(abi_setAll, "(JLjava/util/Collection;)Z"),
		};
		env.register_natives(cls, methods);
	}
};

