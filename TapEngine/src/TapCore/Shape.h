#pragma once
namespace Tap
{
	enum SHAPE_TYPE
	{
		SHAPE_NONE = 0,
		SHAPE_SPHERE = 1,
		SHAPE_BOX = 2,
		SHAPE_PLANE = 3,
		//...
	};

	// 形状描述结构,用于生成
	struct SShapeDesc
	{
		SShapeDesc(SHAPE_TYPE type);
		virtual void Init();	//保持多态类型,但是实际上还没有用到的地方
		SHAPE_TYPE shape_type;
	};

    class CShape
	{
    public:
        CShape(SShapeDesc* shape_desc);
		SHAPE_TYPE GetShapeType() const { return m_ShapeType; }
		virtual void TmpFunc() {}
    private:
        SHAPE_TYPE m_ShapeType;
    };
}
