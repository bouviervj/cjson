#ifndef __CJSON_SMART_POINTER__
#define __CJSON_SMART_POINTER__


namespace cjson {


	// this is a simplified version of smart pointer
	// not shared pointer with counter etc ... simply releasing data	
	template<typename TYPE>
	class smart_ptr {

		private:

			TYPE* _data;
			void(*release)(TYPE*);

		public:
			smart_ptr(TYPE* iData, void (*iRelease) (TYPE*) = NULL){
				_data = iData;
				release = iRelease;
			}

			~smart_ptr(){
				if (release!=NULL) {
					(*release)(_data);
				} else {
					free(_data);
				}
			}

			TYPE* get(){
				return _data;	
			}

	};

}


#endif // __CJSON_SMART_POINTER__
