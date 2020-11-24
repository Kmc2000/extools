#include "exmap.h"
#include "vector.h"

#include "../core/core.h"
#include "../dmdism/opcodes.h"

#include <cmath>
#include <chrono>
#include <memory>

int str_id_vector_pointer;
int str_id_vector_x;
int str_id_vector_y;

//Memory handling stuff.

std::shared_ptr<vector> &get_vector(Value val)
{
	uint32_t v = val.get_by_id(str_id_vector_pointer).value;
	if (v == 0) Runtime("Vector has null extools pointer!");
	return *((std::shared_ptr<vector>*)v);
}

trvh vector_register(unsigned int args_len, Value* args, Value src)
{
	std::shared_ptr<vector> *ptr = new std::shared_ptr<vector>;
	*ptr = std::make_shared<vector>(src.get_by_id(str_id_vector_x).valuef, src.get_by_id(str_id_vector_y).valuef);
	SetVariable(src.type, src.value, str_id_vector_pointer, Value(NUMBER, (int)ptr));
	return Value::Null();
}

trvh vector_unregister(unsigned int args_len, Value* args, Value src)
{
	uint32_t v = src.get_by_id(str_id_vector_pointer).value;
	if (v != 0) {
		std::shared_ptr<vector>* gm = (std::shared_ptr<vector>*)v;
		delete gm;
		SetVariable(src.type, src.value, str_id_vector_pointer, Value::Null());
	}
	return Value::Null();
}
DelDatumPtr oDelDatum_vector;
void hDelDatum_vector(unsigned int datum_id) {
	RawDatum* datum = Core::GetDatumPointerById(datum_id);
	if (datum != nullptr) {
		std::shared_ptr<vector>* gm = nullptr;
		if (datum->len_vars < 10) { // if it has a whole bunch of vars it's probably not a vector. Vectors shouldnt _need_ more than 3 vars.
			for (int i = 0; i < datum->len_vars; i++) {
				if (datum->vars[i].id == str_id_vector_pointer) {
					gm = (std::shared_ptr<vector>*)datum->vars[i].value.value;
					datum->vars[i].value = Value::Null();
					break;
				}
			}
		}
		else {
			Runtime("Uhhhh");
		}
		if (gm != nullptr) {
			delete gm;
		}
	}
	try {
		oDelDatum_vector(datum_id);
	}
	catch (_exception) {}
	
}

trvh get_x(unsigned int args_len, Value* args, Value src)
{
	return Value(get_vector(src)->get_x());
}

trvh get_y(unsigned int args_len, Value* args, Value src)
{
	return Value(get_vector(src)->get_y());
}

trvh set_x(unsigned int args_len, Value* args, Value src)
{
	get_vector(src)->set_x(args_len > 0 ? args[0].valuef : 0);
	return Value::Null();
}

trvh set_y(unsigned int args_len, Value* args, Value src)
{
	get_vector(src)->set_y(args_len > 0 ? args[0].valuef : 0);
	return Value::Null();
}

//Mathematical helpers...
trvh get_seg_intersect(unsigned int args_len, Value* args, Value src)
{
	List l = List(args[0]);

	float p0_x = l.at(1);
	float p0_y = l.at(2);

	float p1_x = l.at(3);
	float p1_y = l.at(4);

	float p2_x = l.at(5);
	float p2_y = l.at(6);

	float p3_x = l.at(7);
	float p3_y = l.at(8);

	//Runtime(std::to_string(l.at(1)).c_str());
	//Runtime(std::to_string(l.at(1)).c_str());


	float s10_x = p1_x - p0_x;
	float s10_y = p1_y - p0_y;
	float s32_x = p3_x - p2_x;
	float s32_y = p3_y - p2_y;

	float denom = s10_x * s32_y - s32_x * s10_y;

	if (denom <= 0) { //No intersection, theyre colinear
		return Value::Null();
	}

	bool denom_is_positive = denom > 0.0;

	float s02_x = p0_x - p2_x;
	float s02_y = p0_y - p2_y;


	float s_numer = s10_x * s02_y - s10_y * s02_x;

	if (s_numer < 0 == denom_is_positive) { //No collision here.
		return Value::Null();
	}

	float t_numer = s32_x * s02_y - s32_y * s02_x;

	
	if (t_numer < 0 == denom_is_positive) { //Annnnd no collision found.
		return Value::Null();
	}

	/*
	if (((s_numer > denom) == denom_is_positive) || ((t_numer > denom) == denom_is_positive)) { //Nope, nothing here.
		return Value::Null();
	}
	*/



	//If we get this far, we've collided!
	float t = t_numer / denom;
	List out = CreateList(0);
	out.append(p0_x + (t * s10_x));
	out.append(p0_y + (t * s10_y));
	return out;
	
}


const char* enable_exmap(){
	oDelDatum_vector = (DelDatumPtr)Core::install_hook((void*)DelDatum, (void*)hDelDatum_vector);
	str_id_vector_pointer = Core::GetStringId("_extools_pointer_vector", true);
	str_id_vector_x = Core::GetStringId("x", true);
	str_id_vector_y = Core::GetStringId("y", true);
	//Vector hooks
	Core::get_proc("/datum/vector2d/proc/vector_register").hook(vector_register);
	Core::get_proc("/datum/vector2d/proc/vector_unregister").hook(vector_unregister);
	Core::get_proc("/datum/vector2d/proc/__get_x").hook(get_x);
	Core::get_proc("/datum/vector2d/proc/__get_y").hook(get_y);
	Core::get_proc("/datum/vector2d/proc/__set_x").hook(set_x);
	Core::get_proc("/datum/vector2d/proc/__set_y").hook(set_y);
	//Math bits
	Core::get_proc("/datum/vector2d/proc/__get_seg_intersect").hook(get_seg_intersect);
    return "ok";
}