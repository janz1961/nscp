#pragma once

#include <map>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>

#include "lua_wrappers.hpp"

#include <scripts/functions.hpp>
#include <nscapi/nscapi_core_helper.hpp>

#ifdef HAVE_LUA_PB
#include <plugin.pb-lua.h>
#endif
namespace script_wrapper {

	typedef lua_wrappers::lua_wrapper lua_wrapper;
	typedef lua_wrappers::lua_script_instance script_instance;
	typedef lua_wrappers::lua_instance_manager instance_manager;
	typedef boost::shared_ptr<lua_wrappers::lua_script_instance> script_instance_type;


	class base_script_object : boost::noncopyable {
	private:
		instance_manager::script_instance_type instance;

	public:
		base_script_object(lua_State *L) {
			instance = instance_manager::get_script(L);
		}
		script_instance_type get_instance() {
			script_instance_type inst = instance.lock();
			if (!inst)
				throw lua_wrappers::LUAException("Invalid instance");
			return inst;
		}
	};

	class core_wrapper : public base_script_object {
	public:
		core_wrapper(lua_State *L) : base_script_object(L) {
			NSC_DEBUG_MSG(_T("get: "));
		}

		static const char className[];
		static const Luna<core_wrapper>::RegType methods[];

		int simple_query(lua_State *L) {
			lua_wrappers::lua_wrapper lua(L);
			try {
				int nargs = lua.size();
				if (nargs == 0)
					return lua.error("nscp.execute requires at least 1 argument!");
				const unsigned int argLen = nargs-1;
				std::list<std::wstring> arguments;
				for (unsigned int i=argLen; i>0; i--) {
					if (lua.is_table()) {
						std::list<std::wstring> table = lua.pop_array();
						arguments.insert(arguments.begin(), table.begin(), table.end());
					} else {
						arguments.push_front(lua.pop_string());
					}
				}
				std::wstring command = lua.pop_string();
				std::wstring message;
				std::wstring perf;
				NSCAPI::nagiosReturn ret = nscapi::core_helper::simple_query(command, arguments, message, perf);
				lua.push_code(ret);
				lua.push_string(message);
				lua.push_string(perf);
				return lua.size();
			} catch (...) {
				return lua.error("Unknown exception in: simple_query");
			}
		}
		int query(lua_State *L) {
			lua_wrappers::lua_wrapper lua(L);
			try {
				int nargs = lua.size();
				if (nargs != 2)
					return lua.error("nscp.query requires 2 arguments!");
				std::string data = utf8::cvt<std::string>(lua.pop_string());
				std::wstring command = lua.pop_string();
				std::string response;
				NSCAPI::nagiosReturn ret = nscapi::plugin_singleton->get_core()->query(command, data, response);
				lua.push_code(ret);
				lua.push_raw_string(response);
				return lua.size();
			} catch (...) {
				return lua.error("Unknown exception in: simple_query");
			}
		}
		int simple_exec(lua_State *L) {
			lua_wrappers::lua_wrapper lua(L);
			try {
				// simple_exec(target, command, arguments)
				if (lua.size() > 1)
					return lua.error("Incorrect syntax: simple_exec(target, command, arguments)");
				std::list<std::wstring> arguments = lua.pop_array();
				std::wstring command = lua.pop_string();
				std::wstring target = lua.pop_string();
				std::list<std::wstring> result;
				NSCAPI::nagiosReturn ret = nscapi::core_helper::exec_simple_command(target, command, arguments, result);
				lua.push_code(ret);
				lua.push_array(result);
				return 2;
			} catch (...) {
				return lua.error("Unknown exception in: simple_query");
			}
		}
		int exec(lua_State *L) {
			lua_wrappers::lua_wrapper lua(L);
			NSC_LOG_ERROR_STD(_T("Unsupported API called: exec"));
			return lua.error("Unsupported API called: exec");
		}
		int simple_submit(lua_State *L) {
			lua_wrappers::lua_wrapper lua(L);
			try {
				// simple_submit(target, command, arguments)
				if (lua.size() != 5)
					return lua.error("Incorrect syntax: simple_submit(channel, command, code, message, perf)");
				std::wstring perf = lua.pop_string();
				std::wstring message = lua.pop_string();
				NSCAPI::nagiosReturn code = lua.pop_code();
				std::wstring command = lua.pop_string();
				std::wstring channel = lua.pop_string();
				std::wstring result;
				NSCAPI::nagiosReturn ret = nscapi::core_helper::submit_simple_message(channel, command, code, message, perf, result);
				lua.push_code(ret);
				lua.push_string(result);
				return 2;
			} catch (...) {
				return lua.error("Unknown exception in: simple_query");
			}
		}
		int submit(lua_State *L) {
			lua_wrappers::lua_wrapper lua(L);
			NSC_LOG_ERROR_STD(_T("Unsupported API called: submit"));
			return lua.error("Unsupported API called: submit");
		}
		int reload(lua_State *L) {
			lua_wrappers::lua_wrapper lua(L);
			if (lua.size() > 1)
				return lua.error("Incorrect syntax: reload([<module>]);");
			std::wstring module = _T("module");
			if (lua.size() == 1)
				module = lua.pop_string();
			get_instance()->get_core()->reload(module);
			return 0;
		}
		int log(lua_State *L) {
			lua_wrappers::lua_wrapper lua(L);
			// log([level], message)
			if (lua.size() > 2 || lua.size() < 1)
				return lua.error("Incorrect syntax: log([<level>], <message>);");
			std::wstring level = _T("info");
			std::wstring message;
			message = lua.pop_string();
			if (lua.size() > 0)
				level = lua.pop_string();
			get_instance()->get_core()->log(nscapi::logging::parse(level), __FILE__, __LINE__, message);
			return 0;
		}
	};

	const char core_wrapper::className[] = "Core";
	const Luna<core_wrapper>::RegType core_wrapper::methods[] = {
		{ "simple_query", &core_wrapper::simple_query },
		{ "query", &core_wrapper::query },
		{ "simple_exec", &core_wrapper::simple_exec },
		{ "exec", &core_wrapper::exec },
		{ "simple_submit", &core_wrapper::simple_submit },
		{ "submit", &core_wrapper::submit },
		{ "reload", &core_wrapper::reload },
		{ "log", &core_wrapper::log },
		{ 0 }
	};

	class registry_wrapper : public base_script_object {
	private:

	public:

		registry_wrapper(lua_State *L) : base_script_object(L) {}

		static const char className[];
		static const Luna<registry_wrapper>::RegType methods[];

		boost::optional<int> read_registration(lua_wrapper &lua, std::wstring &command, int &objref, int &funref, std::wstring &description) {
			// ...(name, function, description)
			// ...(name, instance, function, description)
			std::wstring funname;
			int count = lua.size();
			if (count < 2 && count > 4)
				return lua.error("Invalid number of arguments: " + strEx::s::xtos(lua.size()) + " expected 2-4 arguments");
			if (count > 2 && !lua.pop_string(description)) {
				return lua.error("Invalid description");
			}
			if (lua.pop_string(funname)) {
				lua.getglobal(funname);
			}
			if (!lua.pop_function_ref(funref))
				return lua.error("Invalid function");
			if (count > 3) {
				if (!lua.pop_instance_ref(objref))
					return lua.error("Invalid object");
			}
			if (!lua.pop_string(command))
				return lua.error("Invalid command");
			return boost::optional<int>();
		}
		int register_function(lua_State *L) {
			// void = (cmd, function, desc)
			std::wstring command, description;
			int funref = 0, objref = 0;
			lua_wrapper lua(L);
			boost::optional<int> error = read_registration(lua, command, objref, funref, description);
			if (error)
				return *error;

			if (description.empty()) 
				description = _T("Lua script: ") + command;
			get_instance()->get_core()->registerCommand(get_instance()->get_plugin_id(), command, description);
			get_instance()->get_registry()->register_query(command, get_instance(), objref, funref, false);
			return 0;
		}
		int register_simple_function(lua_State *L) {
			// void = (cmd, function, desc)
			std::wstring command, description;
			int funref = 0, objref = 0;
			lua_wrapper lua(L);
			boost::optional<int> error = read_registration(lua, command, objref, funref, description);
			if (error)
				return *error;

			if (description.empty()) 
				description = _T("Lua script: ") + command;
			get_instance()->get_core()->registerCommand(get_instance()->get_plugin_id(), command, description);
			get_instance()->get_registry()->register_query(command, get_instance(), objref, funref, true);
			return 0;
		}
		int register_cmdline(lua_State *L) {
			lua_wrappers::lua_wrapper lua(L);
			NSC_LOG_ERROR_STD(_T("Unsupported API called: exec"));
			return lua.error("Unsupported API called: exec");
		}
		int register_simple_cmdline(lua_State *L) {
			lua_wrapper lua(L);
			std::wstring name;
			if (lua.is_string()) {
				name = lua.pop_string();
				lua_getglobal(L, utf8::cvt<std::string>(name).c_str());
			}
			if (!lua.is_function())
				return lua.error("Invalid argument not a function: " + utf8::cvt<std::string>(name));

			int func_ref = luaL_ref(L, LUA_REGISTRYINDEX);

			if (func_ref == 0)
				return lua.error("Invalid function: " + utf8::cvt<std::string>(name));
			std::wstring script = lua.pop_string();
			get_instance()->get_registry()->register_exec(script, get_instance(), func_ref);
			return 0;
		}
		int subscription(lua_State *L) {
			lua_wrappers::lua_wrapper lua(L);
			NSC_LOG_ERROR_STD(_T("Unsupported API called: exec"));
			return lua.error("Unsupported API called: exec");
		}
		int simple_subscription(lua_State *L) {
			lua_wrapper lua(L);
			std::wstring name;
			if (lua.is_string()) {
				name = lua.pop_string();
				lua_getglobal(L, utf8::cvt<std::string>(name).c_str());
			}
			if (!lua.is_function())
				return lua.error("Invalid argument not a function: " + utf8::cvt<std::string>(name));

			int func_ref = luaL_ref(L, LUA_REGISTRYINDEX);

			if (func_ref == 0)
				return lua.error("Invalid function: " + utf8::cvt<std::string>(name));
			std::wstring channel = lua.pop_string();
			get_instance()->get_core()->registerSubmissionListener(get_instance()->get_plugin_id(), channel);
			get_instance()->get_registry()->register_subscription(channel, get_instance(), func_ref);
			return 0;
		}
	};

	const char registry_wrapper::className[] = "Registry";
	const Luna<registry_wrapper>::RegType registry_wrapper::methods[] = {
		{ "query", &registry_wrapper::register_function },
		{ "simple_query", &registry_wrapper::register_simple_function },
		{ "cmdline", &registry_wrapper::register_cmdline },
		{ "simple_cmdline", &registry_wrapper::register_simple_cmdline },
		{ "subscription", &registry_wrapper::subscription },
		{ "simple_subscription", &registry_wrapper::simple_subscription },
		{ 0 }
	};



	class settings_wrapper : public base_script_object {
	public:

		settings_wrapper(lua_State *L) : base_script_object(L) {
			NSC_DEBUG_MSG(_T("create"));
		}

		static const char className[];
		static const Luna<settings_wrapper>::RegType methods[];

		int get_section(lua_State *L) {
			lua_wrapper lua(L);
			std::wstring v = lua.op_wstring(1);
			try {
				lua.push_array(get_instance()->get_core()->getSettingsSection(v));
			} catch (...) {
				return lua.error("Unknown exception getting section");
			}
			return 1;
		}
		int get_string(lua_State *L) {
			lua_wrapper lua(L);
			if (lua.size() != 3 && lua.size() != 2)
				return lua.error("Invalid syntax: get_string(section, key, [value])");
			std::wstring v;
			if (lua.size() > 2)
				v = lua.pop_string();
			std::wstring k = lua.pop_string();
			std::wstring s = lua.pop_string();
			lua.push_string(get_instance()->get_core()->getSettingsString(s, k, v));
			return 1;
		}
		int set_string(lua_State *L) {
			lua_wrapper lua(L);
			if (lua.size() != 3)
				return lua.error("Invalid syntax: set_string(section, key, value)");
			std::wstring v = lua.pop_string();
			std::wstring k = lua.pop_string();
			std::wstring s = lua.pop_string();
			get_instance()->get_core()->SetSettingsString(s, k, v);
			return 0;
		}
		int get_bool(lua_State *L) {
			lua_wrapper lua(L);
			if (lua.size() != 3 && lua.size() != 2)
				return lua.error("Invalid syntax: get_bool(section, key, [value])");
			bool v = false;
			if (lua.size() > 2)
				v = lua.pop_boolean();
			std::wstring k = lua.pop_string();
			std::wstring s = lua.pop_string();
			lua.push_boolean(get_instance()->get_core()->getSettingsInt(s, k, v?1:0)==1);
			return 1;
		}
		int set_bool(lua_State *L) {
			lua_wrapper lua(L);
			if (lua.size() != 3)
				return lua.error("Invalid syntax: set_bool(section, key, value)");
			bool v = lua.pop_boolean();
			std::wstring k = lua.pop_string();
			std::wstring s = lua.pop_string();
			get_instance()->get_core()->SetSettingsInt(s, k, v?1:0);
			return 0;
		}
		int get_int(lua_State *L) {
			lua_wrapper lua(L);
			if (lua.size() != 3 && lua.size() != 2)
				return lua.error("Invalid syntax: get_int(section, key, [value])");
			int v = 0;
			if (lua.size() > 2)
				v = lua.pop_int();
			std::wstring k = lua.pop_string();
			std::wstring s = lua.pop_string();
			lua.push_int(get_instance()->get_core()->getSettingsInt(s, k, v));
			return 1;
		}
		int set_int(lua_State *L) {
			lua_wrapper lua(L);
			if (lua.size() != 3)
				return lua.error("Invalid syntax: set_int(section, key, value)");
			int v = lua.pop_int();
			std::wstring k = lua.pop_string();
			std::wstring s = lua.pop_string();
			get_instance()->get_core()->SetSettingsInt(s, k, v);
			return 0;
		}
		int save(lua_State *L) {
			get_instance()->get_core()->settings_save();
			return 0;
		}
		int register_path(lua_State *L) {
			lua_wrapper lua(L);
			if (lua.size() != 3)
				return lua.error("Invalid syntax: register_path(path, title, description)");
			std::wstring description = lua.pop_string();
			std::wstring title = lua.pop_string();
			std::wstring path = lua.pop_string();
			get_instance()->get_core()->settings_register_path(get_instance()->get_plugin_id(), path, title, description, false);
			return 0;
		}
		NSCAPI::settings_type get_type(std::string stype) {
			if (stype == "string" || stype == "str" || stype == "s")
				return NSCAPI::key_string;
			if (stype == "integer" || stype == "int" || stype == "i")
				return NSCAPI::key_integer;
			if (stype == "bool" || stype == "b")
				return NSCAPI::key_bool;
			NSC_LOG_ERROR_STD(_T("Invalid settings type"));
			return NSCAPI::key_string;
		}

		int register_key(lua_State *L) {
			lua_wrapper lua(L);

			if (lua.size() != 5)
				return lua.error("Invalid syntax: register_key(path, key, type, title, description, default)");
			std::wstring defaultValue = lua.pop_string();
			std::wstring description = lua.pop_string();
			std::wstring title = lua.pop_string();
			NSCAPI::settings_type type = get_type(lua.pop_sstring());
			std::wstring key = lua.pop_string();
			std::wstring path = lua.pop_string();
			get_instance()->get_core()->settings_register_key(get_instance()->get_plugin_id(), path, key, type, title, description, defaultValue, false);
			return 0;
		}
		
	};

	const char settings_wrapper::className[] = "Settings";
	const Luna<settings_wrapper>::RegType settings_wrapper::methods[] = {
		{ "get_section", &settings_wrapper::get_section },
		{ "get_string", &settings_wrapper::get_string },
		{ "set_string", &settings_wrapper::set_string },
		{ "get_bool", &settings_wrapper::get_bool },
		{ "set_bool", &settings_wrapper::set_bool },
		{ "get_int", &settings_wrapper::get_int },
		{ "set_int", &settings_wrapper::set_int },
		{ "save", &settings_wrapper::save },
		{ "register_path", &settings_wrapper::register_path },
		{ "register_key", &settings_wrapper::register_key },
		{ 0 }
	};

	class nsclient_wrapper {
	public:

		static int execute (lua_State *L) {
			core_wrapper core(L);
			return core.simple_query(L);
		}

		static int register_command(lua_State *L) {
			registry_wrapper registry(L);
			return registry.register_simple_function(L);
		}

		static int getSetting (lua_State *L) {
			settings_wrapper sw(L);
			return sw.get_string(L);
		}
		static int getSection (lua_State *L) {
			settings_wrapper sw(L);
			return sw.get_section(L);
		}
		static int info (lua_State *L) {
			return log_any(L, NSCAPI::log_level::info);
		}
		static int error (lua_State *L) {
			return log_any(L, NSCAPI::log_level::error);
		}
		static int log_any(lua_State *L, int mode) {
			lua_wrapper lua(L);
			lua_wrapper::stack_trace trace = lua.get_stack_trace();
			int nargs = lua.size();
			std::wstring str;
			for (int i=0;i<nargs;i++) {
				str += lua.pop_string();
			}
			GET_CORE()->log(mode, utf8::cvt<std::string>(trace.first), trace.second, str);
			return 0;
		}

		static const luaL_Reg my_funcs[];

		static void luaopen(lua_State *L) {
			luaL_register(L, "nscp", my_funcs);
			lua_pop(L, 1);
			Luna<core_wrapper>::Register(L);
			Luna<registry_wrapper>::Register(L);
			Luna<settings_wrapper>::Register(L);
#ifdef HAVE_LUA_PB
			GET_CORE()->log(NSCAPI::log_level::error, "test", 123, "Loading lua pb");
			lua_protobuf_Plugin_open(L);
#endif
		}

	};
	const luaL_Reg nsclient_wrapper::my_funcs[] = {
		{"execute", execute},
		{"info", info},
		{"print", info},
		{"error", error},
		{"register", register_command},
		{"getSetting", getSetting},
		{"getSection", getSection},
		{NULL, NULL}
	};

	class lua_script : public script_instance, public boost::enable_shared_from_this<lua_script>  {
		std::string base_path_;
		lua_script(nscapi::core_wrapper* core, const int plugin_id, boost::shared_ptr<lua_wrappers::lua_registry> registry, const std::string alias, const std::string base_path, const std::string script) 
			: script_instance(core, plugin_id, registry, alias, script), base_path_(base_path) {
		}
	public:
		virtual ~lua_script() {
		}

		static boost::shared_ptr<lua_script> create_instance(nscapi::core_wrapper* core, const int plugin_id, boost::shared_ptr<lua_wrappers::lua_registry> registry, const std::wstring alias, const std::wstring base_path, const std::wstring script) {
			boost::shared_ptr<lua_script> instance(new lua_script(core, plugin_id, registry, utf8::cvt<std::string>(alias), utf8::cvt<std::string>(base_path), utf8::cvt<std::string>(script)));
			if (instance) {
				instance->load();
			}
			return instance;
		}

		void load() {
			lua_wrappers::lua_instance_manager::set_script(get_lua_state(), shared_from_this());
			lua_wrappers::lua_wrapper lua(get_lua_state());
			lua.openlibs();
			nsclient_wrapper::luaopen(get_lua_state());
			lua.append_path(base_path_ + "\\scripts\\lua\\lib\\?.lua;" + base_path_ + "scripts\\lua\\?;");
			if (lua.loadfile(get_script()) != 0)
				throw lua_wrappers::LUAException(_T("Failed to load script: ") + get_wscript() + _T(": ") + lua.pop_string());
			if (lua.pcall(0, 0, 0) != 0)
				throw lua_wrappers::LUAException(_T("Failed to execute script: ") + get_wscript() + _T(": ") + lua.pop_string());
		}
		std::wstring get_wscript() const {
			return utf8::cvt<std::wstring>(get_script());
		}
		void unload() {
			lua_wrappers::lua_wrapper lua(get_lua_state());
			lua.gc(LUA_GCCOLLECT, 0);
			lua_wrappers::lua_instance_manager::remove_script(shared_from_this());
		}
		void reload() {
			unload();
			load();
		}
	};
}
